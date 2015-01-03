/*
 ============================================================================
 Name        : sockets-api.c
 Author      : 
 Version     :
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

#define PUERTO "6667"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

/*
 * 		SOCKETS
 */

//Funcion para crear un socket servidor
int crearSocketServidor(char* puertoCliente)
{

  struct addrinfo hints;
  struct addrinfo *serverInfo;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // No importa si uso IPv4 o IPv6
  hints.ai_flags = AI_PASSIVE;        // Asigna el address del localhost: 127.0.0.1
  hints.ai_socktype = SOCK_STREAM;    // Indica que usaremos el protocolo TCP

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
}

//Funcion para crear un socket cliente
int crearSocketCliente(char* ipServidor, char* puertoServidor)
{
  struct addrinfo hints;
  struct addrinfo *ServerInfo;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
  hints.ai_socktype = SOCK_STREAM;    // Indica que usaremos el protocolo TCP

  getaddrinfo(ipServidor, puertoServidor, &hints, &ServerInfo);   // Carga en serverInfo los datos de la conexion

  int serverSocket;
  serverSocket = socket(ServerInfo->ai_family, ServerInfo->ai_socktype, ServerInfo->ai_protocol);
  if(serverSocket==-1)
  {
      printf("Fallo en el socket, se espera 1 minuto y se vuelve a intentar");
      sleep(6);
      serverSocket = socket(ServerInfo->ai_family, ServerInfo->ai_socktype, ServerInfo->ai_protocol);
  }
  connect(serverSocket, ServerInfo->ai_addr, ServerInfo->ai_addrlen);
  freeaddrinfo(ServerInfo);   // No lo necesitamos mas

  return serverSocket;
}

/*Funcion BLOQUEANTE que se mantiene ejecutando hasta recibir una conexion
 *
 * Retorna la estructura de los datos del cliente que se quiere conectar
 */
struct sockaddr_in escucharConexiones(int listeningSocket)
{
	listen(listeningSocket,BACKLOG);
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	return addr;
}

int aceptarConexion(struct sockaddr_in client, int listeningSocket)
{
	socklen_t clientSize = sizeof(client);
	int socketCliente = accept(listeningSocket,(struct sockaddr *) &client, &clientSize);
	return socketCliente; //Nuevo socket de conexion con cliente
}

/*
 * 		LOG FILES
 */

//Crear un archivo de log
t_log* crearLogFile(char* nombreArchivo, char* nomArchSalida)
{
	t_log* logger = log_create(nomArchSalida,nombreArchivo,1,LOG_LEVEL_INFO);
	return logger;
}


//ToDo hacer dsps lo demas que falta

