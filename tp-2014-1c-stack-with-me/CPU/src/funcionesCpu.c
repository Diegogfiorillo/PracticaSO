// FUNCIONES SOBRE EL PROCESO

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>
#include "kernel.h"
#include "funcionesCpu.h"


#define MAXDATASIZE 100

extern int clienteKernelCPU;
extern int clienteUMVCPU;
extern PCB* pcbAUtilizar;
extern t_log* logger;

PCB* crearPcbVacio(){
		PCB* unPcb=malloc(sizeof(PCB));
		unPcb->processID=0;
		unPcb->progCounter=0;
		unPcb->stackSegment=0;
		unPcb->codSegment=0;
		unPcb->etiq_Segment=0;
		unPcb->cursor_stack=0;
		unPcb->cantVariables=0;
		unPcb->codIndex=0;
		unPcb->tam_indice_etiquetas=0;
		unPcb->peso=0;
		unPcb->socketPrograma=0;
		unPcb->prioridad=0;
		return unPcb;
	}

int recibirQuantum(int clienteSocket){

	int quantum;
	if(recv(clienteSocket, &(quantum), sizeof(quantum) , 0)==-1){
		log_error(logger, "Error al recibir el quantum");
	}
	return quantum;

}

char* recibirPCB(){

	char * pcbSerializado = malloc(12*sizeof(uint32_t)); //+sizeof(u_int32_t));
	if(recv(clienteKernelCPU, pcbSerializado, sizeof(pcbSerializado), 0)==-1){
		log_error(logger, "Error al recibir el PCB");
	}
	return pcbSerializado;

}

//serializacion en primitivas con UMV	//creo q ok

char* serializarIdentificadoryPosicion (u_int32_t digito, char identificador, u_int32_t base, t_puntero offset,u_int32_t proceso){

	char * infoSerializada = malloc(4* sizeof(u_int32_t)+sizeof(char));
	int offsetAca = 0;
	int size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offsetAca, &(digito), size_to_send);
	offsetAca += size_to_send;

	size_to_send =  sizeof(char);
	memcpy(infoSerializada + offsetAca, &identificador, size_to_send);
	offsetAca += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offsetAca, &(base), size_to_send);
	offsetAca += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offsetAca, &(offset), size_to_send);
	offsetAca += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offsetAca, &(proceso), size_to_send);
	return infoSerializada;

}

char* serializarPosicionyValor(u_int32_t base, t_valor_variable valor, t_puntero offset){

	int corrimiento=0;
	char *paqueteSerializado=malloc(3* sizeof(u_int32_t));
	memcpy(paqueteSerializado, &(base), sizeof(base));
	corrimiento += (sizeof(u_int32_t));
	memcpy(paqueteSerializado + corrimiento, &(valor), sizeof(valor));
	corrimiento += (sizeof(u_int32_t));
	memcpy(paqueteSerializado + corrimiento, &(offset), sizeof(offset));
	return paqueteSerializado;

}

//manejo de la linea ejecutable

est_linea obtenerLinea()
{
	est_linea estLinea;
	int digito = 1;
	char* linea = leerDatos(digito, pcbAUtilizar->codIndex, (pcbAUtilizar->progCounter)*sizeof(estLinea), sizeof(estLinea),pcbAUtilizar->processID);
	memcpy(&(estLinea.offset), linea, sizeof(uint32_t));
	memcpy(&(estLinea.longitud), linea+sizeof(uint32_t), sizeof(uint32_t));
	return estLinea;

}

char* serializarInfoLinea(u_int32_t base, u_int32_t desplazamiento, u_int32_t longitud, u_int32_t proceso){

	int digito = 1;
	char* infoSerializada = malloc(5*sizeof(u_int32_t));
	int offset = 0;
	int size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(digito), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(base), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(desplazamiento), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(longitud), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(proceso), size_to_send);
	return infoSerializada;

}

void enviarInfoLinea(char* infoLinea){

	int tam = strlen(infoLinea);
	if(send(clienteUMVCPU, infoLinea, tam, 0)==-1){
		log_error(logger, "Error al enviar datos de la linea");
	}

}

Puerto* crear_estructura_config(char* path) {
	t_config* archConfig = config_create(path);
	Puerto* config = malloc(sizeof(Puerto));
	config->ipU = config_get_string_value(archConfig, "IPUMV");
	config->puertoU = config_get_string_value(archConfig, "PUERTOUMV");
	config->ipK = config_get_string_value(archConfig, "IPKERNEL");
	config->puertoK = config_get_string_value(archConfig, "PUERTOKERNEL");
	return config;
}

int sendall(int s, char* buffer, int* len) //me aseguro que se envie toda la informacion
{
	int total = 0;
	int bytesleft = *len;
	int n;
	while (total < *len) {
		n = send(s, buffer + total, bytesleft, 0);
		if (n == -1)
			break;
		total += n;
		bytesleft -= n;
	}
	*len = total;
	return n == -1 ? -1 : 0;
}

int recibir_y_deserializar_PCB()
{
	int size_entero = sizeof(u_int32_t);
	int status;
	int offset = 0;
	char* buffer = malloc(sizeof(PCB));

	status = recv(clienteKernelCPU,buffer,sizeof(PCB),0);
	if(!status) return 0;

	memcpy(&(pcbAUtilizar->processID),buffer,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->progCounter),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->stackSegment),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->codSegment),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->etiq_Segment),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->cursor_stack),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->cantVariables),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->codIndex),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->tam_indice_etiquetas),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->peso),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->socketPrograma),buffer+offset,size_entero);
	offset+=size_entero;
	memcpy(&(pcbAUtilizar->prioridad),buffer+offset,size_entero);
	offset+=size_entero;

	return status;
}

PCB* deserializarPCB(char* package)
{
	PCB* pcb = malloc(sizeof(PCB));
	int offset = 0;
	int size_entero = sizeof(u_int32_t);

	memcpy(&(pcb->processID), package + offset, size_entero);			//saco el id proceso

	offset += size_entero;

	memcpy(&(pcb->progCounter), package + offset, size_entero);		//saco el prog counter

	offset += size_entero;

	memcpy(&(pcb->stackSegment), package + offset, size_entero);		//saco stacksegment

	offset += size_entero;

	memcpy(&(pcb->codSegment), package + offset, size_entero);			//saco cod segment

	offset += size_entero;

	memcpy(&(pcb->etiq_Segment), package + offset, size_entero);		//saco etiq segment

	offset += size_entero;

	memcpy(&(pcb->cursor_stack), package + offset, size_entero);		//saco el cursor del stack

	offset += size_entero;

	memcpy(&(pcb->cantVariables), package + offset, size_entero);		//saco cant de variables

	offset += size_entero;

	memcpy(&(pcb->codIndex), package + offset, size_entero);	//saco el contexto actual

	offset += size_entero;

	memcpy(&(pcb->tam_indice_etiquetas), package + offset, size_entero);		//saco tamaño indice

	offset += size_entero;

	memcpy(&(pcb->peso), package + offset, size_entero);	//saco peso

	offset += size_entero;

	memcpy(&(pcb->socketPrograma), package + offset, size_entero);   //saco socket

	offset += size_entero;

	memcpy(&(pcb->prioridad), package + offset, size_entero);   //saco prioridad

	return pcb;

}

int crearSocketCliente(char* ipServidor, char* puertoServidor) {
	struct addrinfo hints;
	struct addrinfo *ServerInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;// Indica que usaremos el protocolo TCP

	getaddrinfo(ipServidor, puertoServidor, &hints, &ServerInfo);// Carga en serverInfo los datos de la conexion

	int serverSocket;
	serverSocket = socket(ServerInfo->ai_family, ServerInfo->ai_socktype,
			ServerInfo->ai_protocol);
	if (serverSocket == -1) {
		printf(
				"Fallo en el socket, se espera 1 minuto y se vuelve a intentar");
		sleep(6);
		serverSocket = socket(ServerInfo->ai_family,
				ServerInfo->ai_socktype, ServerInfo->ai_protocol);
	}
	connect(serverSocket, ServerInfo->ai_addr, ServerInfo->ai_addrlen);
	freeaddrinfo(ServerInfo);	// No lo necesitamos mas

	return serverSocket;
}

