/*
 * BiblioMensajes.c
 *
 *  Created on: 06/06/2014
 *      Author: mackolou y diegui
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include "BiblioMensajes.h"


#define MAXDATASIZE 100

int handshake(char* paqueteSerializado, int conexionSocket)
{
	int tambuffer= strlen(paqueteSerializado);
	int mensaje = send(conexionSocket, paqueteSerializado, &tambuffer,0);
	if (mensaje!=-1 && mensaje!=0)
	{
		printf("No se pudo enviar el paquete");
	}

	char* contestacion=malloc(50);
	int respuesta= recv(conexionSocket,contestacion,50,0);
	if(respuesta==-1)
	{
		printf("Falló handshake.");
		free(contestacion);
		return -1;
	}
	else
	{
		printf("Handshake realizado con éxito.");
		free(contestacion);
		return 0;
	}

}
int sendall(int s, char* buffer, int* len)			//me aseguro que se envie toda la informacion
{
	int total = 0;
	int bytesleft = *len;
	int n;
	while(total<*len)
	{
		n = send(s, buffer+total, bytesleft, 0);
		if(n==-1) break;
		total+=n;
		bytesleft-=n;
	}
	*len = total;
	return n==-1?-1:0;
}


int crearSocketCliente(char* ipServidor, char* puertoServidor)
{
	struct addrinfo hints;
	struct addrinfo *ServerInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(ipServidor, puertoServidor, &hints, &ServerInfo);	// Carga en serverInfo los datos de la conexion

	int serverSocket;
	serverSocket = socket(ServerInfo->ai_family, ServerInfo->ai_socktype, ServerInfo->ai_protocol);
	if(serverSocket==-1)
	{
		printf("Fallo en el socket, se espera 1 minuto y se vuelve a intentar");
		sleep(60);
		serverSocket = socket(ServerInfo->ai_family, ServerInfo->ai_socktype, ServerInfo->ai_protocol);
	}
	connect(serverSocket, ServerInfo->ai_addr, ServerInfo->ai_addrlen);
	freeaddrinfo(ServerInfo);	// No lo necesitamos mas

	return serverSocket;
}

void deserializarPCB(char **package)
{
	PCB *pcb;
	int offset = 0;

	memcpy(&pcb->digito, *package, sizeof(pcb->digito));					//saco el digito

	offset += sizeof(pcb->digito);

	memcpy(&pcb->processID, *package + offset, sizeof(pcb->processID));			//saco el id proceso

	offset += sizeof(pcb->processID);

	memcpy(&pcb->progCounter, *package + offset, sizeof(pcb->progCounter));		//saco el prog counter

	offset += sizeof(pcb->progCounter);

	memcpy(&pcb->stackSegment, *package + offset, sizeof(pcb->stackSegment));		//saco stacksegment

	offset += sizeof(pcb->stackSegment);

	memcpy(&pcb->codSegment, *package + offset, sizeof(pcb->codSegment));			//saco cod segment

	offset += sizeof(pcb->codSegment);

	memcpy(&pcb->etiq_Segment, *package + offset, sizeof(pcb->etiq_Segment));		//saco etiq segment

	offset += sizeof(pcb->etiq_Segment);

	memcpy(&pcb->cursor_stack, *package + offset, sizeof(pcb->cursor_stack));		//saco el cursor del stack

	offset += sizeof(pcb->cursor_stack);

	memcpy(&pcb->cantVariables, *package + offset, sizeof(pcb->cantVariables));		//saco cant de variables

	offset += sizeof(pcb->cantVariables);

	memcpy(&pcb->contextoActual, *package + offset, sizeof(pcb->contextoActual));	//saco el contexto actual

	offset += sizeof(pcb->contextoActual);

	memcpy(&pcb->tam_indice_etiquetas, *package + offset, sizeof(pcb->tam_indice_etiquetas));		//saco tamaño indice

	offset += sizeof(pcb->tam_indice_etiquetas);

	memcpy(&pcb->peso, *package + offset, sizeof(pcb->peso));					//saco peso

}

void deserializarUMV(char **package)
{
	estructuraUMV *paquete;
	int offset = 0;

	memcpy(&paquete->punteroCodigo, *package, sizeof(paquete->punteroCodigo));					//saco el puntero a codigo

	offset += sizeof(paquete->punteroCodigo);

	memcpy(&paquete->punteroEtiq, *package + offset, sizeof(paquete->punteroEtiq));			//saco el puntero indice etiq

	offset += sizeof(paquete->punteroEtiq);

	memcpy(&paquete->codIndex, *package + offset, sizeof(paquete->codIndex));		//saco el puntero indice codigo

	offset += sizeof(paquete->codIndex);

	memcpy(&paquete->stack, *package + offset, sizeof(paquete->stack));		//saco el indice de stack

}

