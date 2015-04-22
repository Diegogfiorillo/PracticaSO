// FUNCIONES SOBRE EL KERNEL EN EL PROCESO

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


char* serializarPCB_con_digito(u_int32_t digito, PCB* pcb)

{
	char* pcbserializado = malloc(13 * sizeof(u_int32_t));

	u_int32_t offset = 0;
	u_int32_t size_to_send;										//El orden de serializado es el siguiente:

	size_to_send =  sizeof(u_int32_t);							//Primero, el digito de operacion
	memcpy(pcbserializado + offset, &(digito), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);						//Segundo, el id de proceso
	memcpy(pcbserializado + offset, &(pcb->processID), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);					//Tercero, el program counter
	memcpy(pcbserializado + offset, &(pcb->progCounter), size_to_send);
	offset += size_to_send;


	size_to_send =  sizeof(u_int32_t);					//Cuarto, el segmento de stack
	memcpy(pcbserializado + offset, &(pcb->stackSegment), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);						//Quinto, el segmento de codigo
	memcpy(pcbserializado + offset, &(pcb->codSegment), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);					//Sexto, el segmento de etiquetas
	memcpy(pcbserializado + offset, &(pcb->etiq_Segment), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);					//Septimo, el cursor del stack
	memcpy(pcbserializado + offset, &(pcb->cursor_stack), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);					//Octavo, la cantidad de variables
	memcpy(pcbserializado + offset, &(pcb->cantVariables), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);					//Noveno, el contexto Actual
	memcpy(pcbserializado + offset, &(pcb->codIndex), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);				//Decimo, el indice de etiquetas
	memcpy(pcbserializado + offset, &(pcb->tam_indice_etiquetas), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(pcbserializado + offset, &(pcb->peso), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(pcbserializado + offset, &(pcb->socketPrograma), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(pcbserializado + offset, &(pcb->prioridad), size_to_send);
	offset += size_to_send;

	return pcbserializado;
}
