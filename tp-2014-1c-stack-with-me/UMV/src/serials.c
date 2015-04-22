#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serials.h"
#include "funcionesUMV.h"

#define PACKAGESIZE 1024

int crearSocketServidor(char* puertoCliente)
{
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo(NULL, puertoCliente, &hints, &serverInfo);
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if(listenningSocket==-1)
	{
		printf("Fallo en el socket, se espera 1 minuto y se vuelve a intentar");
		sleep(60);
		listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	}

	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	return listenningSocket;
};

t_deKernelAUMV* recibir_paquete_kernel(int s)   //s es de SOCKET
{
	t_deKernelAUMV* paquete = malloc(sizeof(t_deKernelAUMV));
	paquete->codigo_size=0;
	paquete->indice_etiquetas=0;                  //creo espacio e inicializo las variables int
	paquete->id_proceso=0;
	paquete->instrucciones_size=0;

	int status;

	status = recv(s,&(paquete->id_proceso),sizeof(u_int32_t),0);
	if (!status){
		claveContinuidadKernel=0;
		return 0;
	}

	status = recv(s,&(paquete->codigo_size),sizeof(u_int32_t),0);
	if (!status) return 0;

	if(paquete->codigo_size!=0)
	{

		paquete->codigo = malloc(paquete->codigo_size);
		status = recv(s,paquete->codigo,paquete->codigo_size,0);
		if (!status) return 0;

	}
	else
	{
		paquete->codigo=NULL;
	}
	status = recv(s,&(paquete->instrucciones_size),sizeof(u_int32_t),0);
	if (!status) return 0;

	if(paquete->instrucciones_size!=0)
	{

		paquete->instrucciones = malloc(2*sizeof(u_int32_t)*paquete->instrucciones_size);
		status = recv(s,paquete->instrucciones,2*sizeof(u_int32_t)*paquete->instrucciones_size,0);
		if (!status) return 0;
	}
	else
	{
		paquete->instrucciones=NULL;
	}
	status = recv(s,&(paquete->indice_etiquetas),sizeof(u_int32_t),0);
	if (!status) return 0;

	if(paquete->indice_etiquetas!=0)
	{
		paquete->etiquetas = malloc(paquete->indice_etiquetas);
		status = recv(s,paquete->etiquetas,paquete->indice_etiquetas,0);
		if (!status) return 0;
	}
	else
	{
		paquete->etiquetas=NULL;
	}
	int offset=(4*sizeof(u_int32_t))+ paquete->codigo_size +2*sizeof(u_int32_t)*paquete->instrucciones_size+paquete->indice_etiquetas;
	printf("Recibi %d bytes por el momento\n", offset);
	int falta= 1024- offset;
	printf("Faltan %d bytes mas\n", falta);
	return paquete;
}

char* serializar_punteros_Kernel(char* serializado, u_int32_t a, u_int32_t b, u_int32_t c, u_int32_t d)
{
	int offset = 0;
	int size_to_send = sizeof(u_int32_t);

	memcpy(serializado+offset,&a,size_to_send);
	offset+=size_to_send;

	memcpy(serializado+offset,&b,size_to_send);
	offset+=size_to_send;

	memcpy(serializado+offset,&c,size_to_send);
	offset+=size_to_send;

	memcpy(serializado+offset,&d,size_to_send);
	offset+=size_to_send;

	return serializado;
}

char* serializarVariable(char* serializado, char variable)
{
	memcpy(serializado,&variable, sizeof(char));
	return serializado;
}

char* serializarContenido(char* serializado, int contenido)
{
	memcpy(serializado,&contenido, sizeof(u_int32_t));
	return serializado;
}

char* serializarDireccionCPU(char* serializado, u_int32_t puntero)
{
	int size_to_send = sizeof(u_int32_t);
	memcpy(serializado,&puntero, size_to_send);
	return serializado;
}

t_Dir_deCPUaUMV* recibir_Direccion_CPU(int s)   //s es de SOCKET
{
	t_Dir_deCPUaUMV* paqueteDireccion = malloc(sizeof(t_Dir_deCPUaUMV));
	paqueteDireccion->direccionDelaProximaLineaAEjecutar=0;
	paqueteDireccion->offset=0;
	paqueteDireccion->tamanio=0;
	paqueteDireccion->idDeProceso=0;
	u_int32_t offset=0;
	u_int32_t status;

	status = recv(s,&(paqueteDireccion->direccionDelaProximaLineaAEjecutar),sizeof(u_int32_t),0);
	if (!status) return 0;
	offset+=sizeof(u_int32_t);

	printf("direccion proxima linea %d \n", paqueteDireccion->direccionDelaProximaLineaAEjecutar);

	status = recv(s,&(paqueteDireccion->offset),sizeof(u_int32_t),0);
	if (!status) return 0;
	offset+=sizeof(u_int32_t);

	printf("direccion offset %d \n", paqueteDireccion->offset);

	status = recv(s,&(paqueteDireccion->tamanio),sizeof(u_int32_t),0);
	if (!status) return 0;
	offset+=sizeof(u_int32_t);

	printf("tamanio %d \n", paqueteDireccion->tamanio);

	status = recv(s,&(paqueteDireccion->idDeProceso),sizeof(u_int32_t),0);
	if (!status) return 0;

	printf("idProceso %d \n", paqueteDireccion->idDeProceso);

	return paqueteDireccion;
}

t_Nombre_deCPUaUMV* recibir_Nombre_CPU(int s)   //s es de SOCKET
{
	t_Nombre_deCPUaUMV* paqueteDireccion = malloc(sizeof(t_Nombre_deCPUaUMV));
	paqueteDireccion->offset=0;
	paqueteDireccion->idDeProceso=0;
	u_int32_t offset = 0;
	u_int32_t status;

	status = recv(s,&(paqueteDireccion->nombre_variable),sizeof(char),0);
	if (!status) return 0;
	status = recv(s,&(paqueteDireccion->direccion),sizeof(u_int32_t),0);
	if (!status) return 0;
	status = recv(s,&(paqueteDireccion->offset),sizeof(u_int32_t),0);
	if (!status) return 0;
	status = recv(s,&(paqueteDireccion->idDeProceso),sizeof(u_int32_t),0);
	if (!status) return 0;

	return paqueteDireccion;
}

t_DirYValor_deCPUaUMV* recibir_DirYValor_CPU(int s)   //s es de SOCKET
{
	t_DirYValor_deCPUaUMV* paqueteDirYValor = malloc(sizeof(t_DirYValor_deCPUaUMV));
	paqueteDirYValor->direccion_variable=0;
	paqueteDirYValor->offset=0;
	paqueteDirYValor->modificaion=0;
	paqueteDirYValor->idDeProceso;

	u_int32_t status;

	status = recv(s,&(paqueteDirYValor->direccion_variable),sizeof(u_int32_t),0);
	if (!status) return 0;

	status = recv(s,&(paqueteDirYValor->offset),sizeof(u_int32_t),0);
	if (!status) return 0;

	status = recv(s,&(paqueteDirYValor->modificaion),sizeof(u_int32_t),0);
	if (!status) return 0;

	status = recv(s,&(paqueteDirYValor->idDeProceso),sizeof(u_int32_t),0);
	if (!status) return 0;

	return paqueteDirYValor;
}

t_variableYValor_deCPUaUMV* recibir_VariableYValor_CPU(int s)   //s es de SOCKET
{
	t_variableYValor_deCPUaUMV* paqueteVariableYValor = malloc(sizeof(t_variableYValor_deCPUaUMV));
	paqueteVariableYValor->offset=0;
	paqueteVariableYValor->idDeProceso=0;
	paqueteVariableYValor->direccion_variable=0;
	int offset=sizeof(u_int32_t);
	int status;
	char* codigoSerializado = malloc(PACKAGESIZE);

	status = recv(s,codigoSerializado,sizeof(u_int32_t),0);
	memcpy(&(paqueteVariableYValor->direccion_variable),codigoSerializado+offset,sizeof(char*));
	if (!status) return 0;

	status = recv(s,codigoSerializado,sizeof(u_int32_t),0);
	memcpy(&(paqueteVariableYValor->offset),codigoSerializado+offset,sizeof(u_int32_t));
	if (!status) return 0;

	status = recv(s,codigoSerializado,sizeof(u_int32_t),0);
	memcpy(&(paqueteVariableYValor->idDeProceso),codigoSerializado+offset,sizeof(u_int32_t));
	if (!status) return 0;

	free(codigoSerializado);
	return paqueteVariableYValor;
}

//char* serializarVariableYContenido_CPU(char* serializado, t_variableYValor_deUMVaCPU* punteros)
//{
//	int size_to_send = sizeof(u_int32_t);
//	int offset=0;
//	memcpy(serializado,punteros->contenido, sizeof(char));
//	offset+=sizeof(char);
//	memcpy(serializado,punteros->contenido, size_to_send);
//	return serializado;
//}

 u_int32_t recibirCodigoPeticion (int socket)
{
	u_int32_t codigoPeticion=0;
	u_int32_t status;
	status = recv(socket,&codigoPeticion,sizeof(u_int32_t),0);
	printf("se recibieron %d bytes", status);
	if (!status) return 0;
	return codigoPeticion;
}
