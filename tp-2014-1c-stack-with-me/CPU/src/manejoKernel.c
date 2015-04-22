//FUNCIONES SOBRE EL KERNEL EN LAS PRIMITIVAS

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
#include <parser/parser.h>
#include "manejoKernel.h"
#include "kernel.h"


char* serializarNombreVarCompartida(t_nombre_compartida nombre){

		uint32_t identificador = 7;
		printf("La compartida es %s", nombre);
		uint32_t longitud = strlen(nombre);

		char * infoSerializada = malloc(2*sizeof(u_int32_t)+ longitud);
		int offset = 0;
		int size_to_send;

		size_to_send =  sizeof(u_int32_t);
		memcpy(infoSerializada + offset, &(identificador), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);
		memcpy(infoSerializada + offset, &(longitud), size_to_send);
		offset += size_to_send;

		size_to_send =  longitud;
		memcpy(infoSerializada + offset, nombre, size_to_send);
		offset += size_to_send;


		return infoSerializada;

}

char* serializarNombreyValorVarCompartida(t_nombre_compartida nombre, t_valor_variable valor){

		uint32_t identificador = 6;
		uint32_t longitud = strlen(nombre);

		char * infoSerializada = malloc(3*sizeof(u_int32_t)+ longitud);
		int offset = 0;
		int size_to_send;

		size_to_send =  sizeof(u_int32_t);
		memcpy(infoSerializada + offset, &(identificador), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);
		memcpy(infoSerializada + offset, &(longitud), size_to_send);
		offset += size_to_send;

		size_to_send =  longitud;
		memcpy(infoSerializada + offset, nombre, size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);
		memcpy(infoSerializada + offset, &(valor), size_to_send);

		return infoSerializada;

}

char* serializarValor(t_valor_variable valor, u_int32_t campoDelPCB){

	uint32_t identificador = 11;

	char* infoSerializada = malloc(3*(sizeof(u_int32_t)));
	int offset = 0;
	int size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(identificador), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(valor), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);

	memcpy(infoSerializada + offset, &(campoDelPCB), size_to_send);
	printf("Mando 1: %d\n", identificador);
	printf("Mando 2: %d\n", valor);
	printf("Mando 3: %d\n", campoDelPCB);
	return infoSerializada;
}

char* serializarTexto(char* texto, u_int32_t campoDelPCB){

	uint32_t identificador = 10;
	uint32_t longitud = strlen(texto);

	char * infoSerializada = malloc(3*sizeof(u_int32_t)+longitud);
	int offset = 0;
	int size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(identificador), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(longitud), size_to_send);
	offset += size_to_send;
	size_to_send =  longitud;
	memcpy(infoSerializada + offset, texto, size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(campoDelPCB), size_to_send);
	return infoSerializada;

}

char* serializarTiempoyDispositivo(uint32_t tiempo, t_nombre_dispositivo dispositivo, PCB* pcbcito){

	uint32_t identificador = 3;
	uint32_t longitud = strlen(dispositivo);
	printf("Mando una longitud de: %d\n", longitud);
	printf("Mando un dispositivo: %s\n", dispositivo);
	printf("lo Mando %d tiempo\n", tiempo);

	char* pcbcitoSerializado = serializarPCB(pcbcito);
	char* infoSerializada = malloc(3*sizeof(u_int32_t)+longitud+sizeof(PCB));
	uint32_t offset = 0;
	uint32_t size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(identificador), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(longitud), size_to_send);
	offset += size_to_send;
	size_to_send =  longitud;
	memcpy(infoSerializada + offset, dispositivo, size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(tiempo), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(PCB);
	memcpy(infoSerializada + offset, pcbcitoSerializado, size_to_send);
	return infoSerializada;

}

char* serializarSemaforoWait(t_nombre_semaforo identificador_semaforo, PCB* pcbcito){

	uint32_t identificador = 8;
	uint32_t longitud = strlen(identificador_semaforo);

	char* pcbcitoSerializado = serializarPCB(pcbcito);
	printf("Serializo PCB\n");
	char* infoSerializada = malloc(2*sizeof(u_int32_t)+sizeof(char)+sizeof(PCB));
	u_int32_t offset = 0;
	u_int32_t size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(identificador), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(longitud), size_to_send);
	offset += size_to_send;
	size_to_send =  longitud;
	memcpy(infoSerializada + offset, identificador_semaforo, longitud);
	offset += longitud;
	size_to_send = sizeof(PCB);
	memcpy(infoSerializada + offset, pcbcitoSerializado, size_to_send);
	return infoSerializada;

}

char* serializarSemaforoSignal(t_nombre_semaforo identificador_semaforo){

	uint32_t identificador = 9;
	uint32_t longitud = strlen(identificador_semaforo);

	char * infoSerializada = malloc(2*sizeof(u_int32_t)+strlen(identificador_semaforo));
	u_int32_t offset = 0;
	u_int32_t size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(identificador), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(longitud), size_to_send);
	offset += size_to_send;
	size_to_send =  longitud;
	memcpy(infoSerializada + offset, identificador_semaforo, size_to_send);
	return infoSerializada;

}

char* serializarPCB( PCB* pcb){
	char *pcbserializado = malloc(12*sizeof(u_int32_t));

	u_int32_t offset = 0;
	u_int32_t size_to_send;										//El orden de serializado es el siguiente:

	size_to_send =  sizeof(pcb->processID);						//Segundo, el id de proceso
	memcpy(pcbserializado + offset, &(pcb->processID), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->progCounter);					//Tercero, el program counter
	memcpy(pcbserializado + offset, &(pcb->progCounter), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->stackSegment);					//Cuarto, el segmento de stack
	memcpy(pcbserializado + offset, &(pcb->stackSegment), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->codSegment);						//Quinto, el segmento de codigo
	memcpy(pcbserializado + offset, &(pcb->codSegment), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->etiq_Segment);					//Sexto, el segmento de etiquetas
	memcpy(pcbserializado + offset, &(pcb->etiq_Segment), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->cursor_stack);					//Septimo, el cursor del stack
	memcpy(pcbserializado + offset, &(pcb->cursor_stack), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->cantVariables);					//Octavo, la cantidad de variables
	memcpy(pcbserializado + offset, &(pcb->cantVariables), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->codIndex);					//Noveno, el contexto Actual
	memcpy(pcbserializado + offset, &(pcb->codIndex), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->tam_indice_etiquetas);				//Decimo, el indice de etiquetas
	memcpy(pcbserializado + offset, &(pcb->tam_indice_etiquetas), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->peso);
	memcpy(pcbserializado + offset, &(pcb->peso), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->socketPrograma);
	memcpy(pcbserializado + offset, &(pcb->socketPrograma), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(pcb->prioridad);
	memcpy(pcbserializado + offset, &(pcb->prioridad), size_to_send);
	offset += size_to_send;

	return pcbserializado;
}
