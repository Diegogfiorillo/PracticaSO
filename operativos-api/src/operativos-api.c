/*
 ============================================================================
 Name        : operativos-api.c
 Author      : FC
 Version     : 1.0.0
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
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
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/select.h>
#include "operativos-api.h"
#include "socket.h"

#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar



/*
 * 		LOG FILES
 */

//Crear un archivo de log
t_log* crearLogFile(char* nombreArchivo, char* nomArchSalida) {
	t_log* logger = log_create(nomArchSalida, nombreArchivo, 1, LOG_LEVEL_INFO);
	return logger;
}

/*
 * 		SEMAFOROS
 */

void inicializarSemaforo(sem_t semaforoGlobal, t_log* logger) //semaforoGlobal es la variable definida
{
	int semInicializado = sem_init(&semaforoGlobal, 1, 1);
	if (semInicializado == -1) {
		log_error(logger, "No pudo crearse el semaforo " + semaforoGlobal);
	}

}

void destruirSemaforo(sem_t semaforoGlobal, t_log* logger) {
	sem_destroy(&semaforoGlobal);
	log_info(logger, "Se destruyo el semaforo " + semaforoGlobal);
}

/*
 * 		SERIALIZACION
 */

char* serializar_datos(char* codigoASerializar) {

	char *paqueteSerializado = malloc(strlen(codigoASerializar)); //Malloc del tamaño a guardar

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, strlen(codigoASerializar),
			size_to_send);
	offset += size_to_send;

	size_to_send = strlen(codigoASerializar);
	memcpy(paqueteSerializado + offset, codigoASerializar, size_to_send);
	offset += size_to_send;

	printf(" %s\n", paqueteSerializado); //Chequeamos que este todo ok

	return paqueteSerializado;
}

//Deserializar

/*
 * La convencion de los paquetes serializados es tamanioAtributo1|atributo1
 * sin las barras ni espacios
 */
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
