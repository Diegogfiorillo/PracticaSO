#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <commons/log.h>
#include <signal.h>
#include <pthread.h>
#include "funcionesPrograma.h"
int recibido = 1;
pthread_mutex_t semVariable = PTHREAD_MUTEX_INITIALIZER;
extern int kernelSocket;
extern t_log* logger;

int enviarCodigo(char* buf, u_int32_t* longitud) {
	int total = 0; //cuantos bytes hemos enviado
	int resto = *longitud; //cuantos se han quedado pendientes
	int n;
	while (total < *longitud) {
		n = send(kernelSocket, buf + total, resto, 0);
		if (n == -1) {
			break;
		}
		total += n;
		resto -= n;
	}
	*longitud = total; //devuelve aqui la cantidad enviada en realidad
	return n == -1 ? -1 : 0; // devuelve -1 si hay falla, cero en otro caso
}

char* obtenerPrimerLinea(char* texto) {
	int i = 0;
	char* codigoAnsisop = malloc(strlen(texto));
	while (texto[i] != '\n')
		i++;
	texto[i] = '\0';
	memcpy(codigoAnsisop, texto, strlen(texto));
	return codigoAnsisop;
}

char* serializar_paquete_programa(package* paquete) {

	char *serializado = malloc(paquete->size + sizeof(u_int32_t));

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(paquete->size);
	memcpy(serializado + offset, &(paquete->size), size_to_send);
	offset += size_to_send;

	size_to_send = paquete->size;
	memcpy(serializado + offset, paquete->codigo, size_to_send);
	offset += size_to_send;

	return serializado;
}

package* deserializar_paquete(char* paqueteSerializado) {

	u_int32_t size;
	int offset = 0;
	package* paquete = malloc(sizeof(package));

	memcpy(&size, paqueteSerializado, sizeof(u_int32_t));
	paquete->size = size;

	offset += sizeof(u_int32_t);
	memcpy(paquete->codigo, paqueteSerializado + offset, paquete->size);

	return paquete;
}

void finalizar() {
	log_error(logger, "Se dio de baja el programa por parte del usuario");
	u_int32_t digito=3;
	u_int32_t digitoKernel;
	u_int32_t dobleHandshake = 1;
	int by = send(kernelSocket, &digito, sizeof(u_int32_t), 0);
	if (!by) log_error(logger,"no se pudo enviar la senial al kernel de finalizacion");
	recv(kernelSocket,&digitoKernel,sizeof(u_int32_t),0);
	send(kernelSocket,&dobleHandshake,sizeof(u_int32_t),0);
	close(kernelSocket);
	exit(1);
}

void esperarSentencias() {
	char buffer[MAX_MESSAGE_SIZE];

	int digito;
	sentenciasImprimir* paquete = malloc(sizeof(sentenciasImprimir));
	while (recibido) {

		recibido = recv(kernelSocket,&digito,sizeof(u_int32_t),0);
		if(!recibido){
			log_error(logger, "No se obtuvo respuesta del Kernel\n");
			abort();
		}
		recibido = recibirYDeserializar(digito,buffer, paquete);
	}
	free(paquete);

}

int recibirYDeserializar(int digito, char* buffer, sentenciasImprimir* paquete) {
	int status;
	int tamanio = 0;


	if(digito==1)
	{
		status = recv(kernelSocket, buffer, sizeof(u_int32_t), 0);
		memcpy(&tamanio, buffer, sizeof(u_int32_t));
		if (!status)return 0;
		paquete->texto=malloc(tamanio+1);
		status = recv(kernelSocket, buffer, tamanio, 0);
		memcpy(paquete->texto, buffer, tamanio);
		paquete->texto[tamanio]='\0';
		if (!status)return 0;

		log_info(logger, "Ha llegado del Kernel: %s \n", paquete->texto);
		return 1;
	}
	else if(digito==2)
	{
		int valor;
		status = recv(kernelSocket, &valor, sizeof(u_int32_t), 0);
		if (!status)return 0;
		log_info(logger, "Ha llegado del Kernel: %d \n", valor);
		return 1;
	} else if(digito==3)
	{
		u_int32_t chequeoVerif=1;
		int envioCorrecto = send(kernelSocket, &chequeoVerif, sizeof(u_int32_t), 0);
		if(envioCorrecto<=0) return 0;
		printf("End.\nFinaliza el programa.\n");
		recibido=0;
		return 0;
	} else if(digito==4)
	{
			printf("No hay espacio suficiente en la memoria\n");
	u_int32_t chequeoVerif=1;
		int envioCorrecto = send(kernelSocket, &chequeoVerif, sizeof(u_int32_t), 0);
		if(envioCorrecto<=0) return 0;
		printf("End.\nFinaliza el programa.\n");
			recibido=0;
			return 0;
	}
	else if(digito==5)
	{

		printf(
										"                     ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶                     \n                 ¶¶¶¶¶¶             ¶¶¶¶¶¶¶                \n              ¶¶¶¶                       ¶¶¶¶              \n             ¶¶¶                           ¶¶¶             \n            ¶¶                               ¶¶            \n           ¶¶             WARNING!!          ¶¶           \n          ¶¶            SEGMENTATION           ¶¶          \n          ¶¶ ¶¶            FAULT            ¶¶ ¶¶          \n          ¶¶ ¶¶                             ¶¶ ¶¶          \n          ¶¶ ¶¶           PROGRAM           ¶¶ ¶¶          \n          ¶¶  ¶¶           CLOSED          ¶¶  ¶¶          \n          ¶¶  ¶¶                           ¶¶  ¶¶          \n           ¶¶ ¶¶   ¶¶¶¶¶¶¶¶      ¶¶¶¶¶¶¶   ¶¶ ¶¶           \n            ¶¶¶¶ ¶¶¶¶¶¶¶¶¶¶     ¶¶¶¶¶¶¶¶¶¶ ¶¶¶¶¶           \n             ¶¶¶ ¶¶¶¶¶¶¶¶|¶     ¶¶¶¶¶¶¶¶¶¶ ¶¶¶             \n    ¶¶¶       ¶¶  ¶¶¶¶¶¶¶¶       ¶¶¶¶¶¶¶¶¶ ¶¶        ¶¶¶¶  \n   ¶¶¶¶¶     ¶¶   ¶¶¶¶¶¶¶   ¶ ¶   ¶¶¶¶¶¶¶   ¶¶     ¶¶¶¶¶¶  \n  ¶¶   ¶¶    ¶¶     ¶¶¶    ¶¶¶¶¶    ¶¶¶     ¶¶    ¶¶   ¶¶  \n ¶¶¶    ¶¶¶¶  ¶¶          ¶¶¶ ¶¶¶          ¶¶  ¶¶¶¶    ¶¶¶ \n¶¶         ¶¶¶¶¶¶¶¶       ¶¶¶ ¶¶¶       ¶¶¶¶¶¶¶¶¶        ¶¶\n¶¶¶¶¶¶¶¶¶     ¶¶¶¶¶¶¶¶    ¶¶¶ ¶¶¶    ¶¶¶¶¶¶¶¶      ¶¶¶¶¶¶¶¶\n  ¶¶¶¶ ¶¶¶¶¶      ¶¶¶¶¶            ¶¶¶ ¶¶     ¶¶¶¶¶¶ ¶¶¶   \n          ¶¶¶¶¶¶  ¶¶¶ ¶¶           ¶¶ ¶¶¶  ¶¶¶¶¶¶          \n              ¶¶¶¶¶¶ ¶  ¶¶¶¶¶¶¶¶¶¶¶ ¶¶ ¶¶¶¶¶¶              \n                  ¶¶ ¶ ¶¶ ¶¶ ¶ ¶¶¶¶¶¶¶ ¶¶                  \n                ¶¶¶¶ ¶¶ ¶ ¶¶ ¶ ¶¶ ¶ ¶¶ ¶¶¶¶¶               \n            ¶¶¶¶¶ ¶¶   ¶¶¶¶¶¶¶¶¶¶¶¶¶   ¶¶ ¶¶¶¶¶            \n    ¶¶¶¶¶¶¶¶¶¶     ¶¶                 ¶¶      ¶¶¶¶¶¶¶¶¶    \n   ¶¶           ¶¶¶¶¶¶¶             ¶¶¶¶¶¶¶¶          ¶¶   \n    ¶¶¶     ¶¶¶¶¶     ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶     ¶¶¶¶¶     ¶¶¶    \n      ¶¶   ¶¶¶           ¶¶¶¶¶¶¶¶¶           ¶¶¶   ¶¶      \n      ¶¶  ¶¶                                   ¶¶  ¶¶      \n       ¶¶¶¶                                     ¶¶¶        \n\n");
		u_int32_t chequeoVerif=1;
		int envioCorrecto = send(kernelSocket, &chequeoVerif, sizeof(u_int32_t), 0);
		if(envioCorrecto<=0) return 0;
		printf("End.\nFinaliza el programa.\n");
				recibido=0;
				return 0;
	}
	return -1;
}

