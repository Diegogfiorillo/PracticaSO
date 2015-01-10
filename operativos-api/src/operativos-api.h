/*
 * sockets-api.h
 *
 *  Created on: 29/12/2014
 *      Author: federico
 */

#ifndef SOCKETS_API_H_
#define SOCKETS_API_H_

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
#include <commons/log.h>
#include <commons/config.h>
#include <commons/error.h>
#include <commons/collections/list.h>


//Sockets
int crearSocketServidor(char*);
int crearSocketCliente(char*, char*);
struct sockaddr_in escucharConexiones(int);
int aceptarConexion(struct sockaddr_in, int);

/*
 *
 *ToDo faltan las funciones de enviar y recibir informacion
 *
 */

//Log Files
t_log* crearLogFile(char*,char*);

//Semaforos
void crearSemaforo(sem_t, t_log*);
void destruirSemaforo(sem_t, t_log*);

//Serializacion
char* serializar_datos(char*);
void* deserializar_paquete(char*,int);

