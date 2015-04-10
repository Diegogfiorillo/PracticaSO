/*
 * socket.c
 *
 *  Author: fc
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "socket.h"

/**** FUNCIONES PRIVADAS ****/

sock_t* _create_socket()
{
	sock_t* sock = (sock_t*) malloc(sizeof(sock_t));
	sock->sockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr));

	sock->fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock==-1){
	      sleep(3);
	      printf("Error en la creacion del socket, volviendo a intentar..");
	      sock = socket(AF_INET, SOCK_STREAM, 0);
	  }

	return sock;
}

void _prepare_conexion(sock_t* socket, char* ip, uint32_t puerto)
{
	// Seteamos la IP, si es NULL es localhost
	if(ip == NULL)
		socket->sockaddr->sin_addr.s_addr = htonl(INADDR_ANY); // Conexion Local
	else
		socket->sockaddr->sin_addr.s_addr = inet_addr(ip);	// Conexion Remota

	socket->sockaddr->sin_family = AF_INET;
	socket->sockaddr->sin_port = htons(puerto);
	memset(&(socket->sockaddr->sin_zero), '\0', 8);
}


int32_t _refresh_port(sock_t* socket)
{
	int32_t yes=1;
	return setsockopt(socket->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t));
}


int32_t _bind_port(sock_t* socket)
{
	_refresh_port(socket);
	return bind(socket->fd, (struct sockaddr *)socket->sockaddr, sizeof(struct sockaddr));
}

int32_t _send(sock_t* socket, void* buffer, uint32_t len)
{
	return send(socket->fd,buffer,len,0);
}

int32_t _receive(sock_t* socket, void* buffer, uint32_t len)
{
	return recv(socket->fd, buffer, len, 0);
}

void _close_socket(sock_t* socket)
{
	shutdown(socket->fd, 2);
}

void _free_socket(sock_t* socket)
{
	free(socket->sockaddr);
	free(socket);
}

/**** FUNCIONES PUBLICAS ****/

/* Funcion para crear un socket servidor */

sock_t* create_server_socket(uint32_t puertoCliente)
{

	sock_t* sock = _create_socket();

	_prepare_conexion(sock,NULL,puertoCliente);

	if(_bind_port(sock) == -1)
			return NULL;

	return sock;
}

/* Funcion para crear un socket cliente */

sock_t* create_client_socket(char* ipServidor, uint32_t puertoServidor)
{

	sock_t* clientSocket = _create_socket();

	_prepare_conexion(clientSocket,ipServidor,puertoServidor);

	return clientSocket;

}

/* Conecta un cliente al socket servidor */

int32_t connect(sock_t* socket)
{
	return connect(socket->fd, (struct sockaddr *)socket->sockaddr, sizeof(struct sockaddr));
}

/* El socket servidor escucha conexiones de clientes entrantes */

int32_t listen_connections(sock_t* socket)
{
	return listen(socket->fd, CANTIDAD_CONEXIONES_LISTEN);
}

/* El socket servidor acepta conexion entrante */

sock_t* accept_connection(sock_t* socket)
{
	sock_t* sock_nuevo = _crear_socket();

	uint32_t i = sizeof(struct sockaddr_in);

	sock_nuevo->fd = accept(socket->fd, (struct sockaddr *)sock_nuevo->sockaddr, &i);

	return sock_nuevo->fd == -1?NULL:sock_nuevo;
}

int32_t send_bytes(sock_t* socket, void* buffer, u_int32_t len)          //me aseguro que se envie toda la informacion
{
    int32_t total = 0;
    int32_t bytesLeft = len;
    int32_t n;
    while(total<len)
    {
        n = send(socket->fd, buffer+total, bytesLeft, 0);
        if(n==-1) break;
        total+=n;
        bytesLeft-=n;
    }
    len = total;
    return n==-1?-1:0;
}

int32_t receive_bytes(sock_t* sock, void* bufferSalida, uint32_t lenBuffer)
{
	int32_t n = 0;
	int32_t bytesLeft = lenBuffer;
	int32_t recibido = 0;

	while(recibido < lenBuffer)
	{
		n = _receive(sock,bufferSalida+recibido,lenBuffer-recibido);
		if(n==-1 || n==0) break;
		recibido += n;
		bytesLeft -= n;
	}
	lenBuffer = recibido;
	return (n==-1 || n==0)?-1:0;
}
