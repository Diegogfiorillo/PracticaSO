/*
 * socket.h
 *
 * Author: fc
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define CANTIDAD_CONEXIONES_LISTEN 20
#define CABECERA_VALIDA 666

typedef struct sock
{
	int32_t fd;
	struct sockaddr_in* sockaddr;

} sock_t;

/* FUNCIONES PRIVADAS */

/**
 * Crea un Socket
 */
sock_t* _create_socket();

/**
 * Prepara la conexion para trabajar con el socket.
 * Si es localhost usar NULL en la IP
 */
void _prepare_conexion(sock_t*, char*, uint32_t);

/**
 * Limpia el puerto para reutilizarlo (para los casos en que se corre
 * varias veces un bind sobre el mismo puerto)
 */
int32_t _refresh_port(sock_t*);

/**
 * Bindea el puerto al Socket
 */
int32_t _bind_port(sock_t*);

/**
 * Envia un mensaje a traves del socket
 * Devuelve la cantidad de bytes que se enviaron realmente, o -1 en caso de error
 */
int32_t _send(sock_t*, char*, uint32_t);

/**
 * Lee un mensaje y lo almacena en buff
 * Devuelve la cantidad leia de bytes, 0 en caso de que la conexion este cerrada, o -1 en caso de error
 */
int32_t _receive(sock_t*, void*, uint32_t);

/*
 * Procedemos a cerrar el socket con la opcion SHUT_RDWR (No more receptions or transmissions)
 */
void _close_socket(sock_t* );

/**
 * Liberamos la memoria ocupada por el struct
 */
void _free_socket(sock_t*);


/***FUNCIONES PUBLICAS***/

/**
 * Crea un socket para recibir mensajes.
 * Recibe el puerto de la maquina remota que se va a conectar.
 * NO REALIZA EL LISTEN
 * @RETURNS: El struct socket.
 */
sock_t* create_server_socket(uint32_t);

/**
 * Crea un socket para enviar mensajes.
 * Recibe el puerto de la maquina remota que esta escuchando.
 * NO REALIZA EL CONNECT.
 *
 * @RETURNS: El struct socket.
 */
sock_t* create_client_socket(char*, uint32_t);

/**
 * Conecta con otra PC
 *
 * @RETURNS: -1 en caso de error
 * NOTA: Recordar que es una funcion bloqueante
 */
int32_t connect(sock_t*);

/**
 * Establece el socket para escuchar
 * TODO: Esta funcion podria incluirse en crear_socket_escuchador ya que no es bloqueante
 *
 * @RETURNS: -1 en caso de error
 */
int32_t listen_connections(sock_t*);

/**
 * Acepta una conexion entrante
 *
 * @RETURNS: el nuevo FD; o NULL en caso de error
 * NOTA: Recordar que es una funcion bloqueante
 */
sock_t* accept_connection(sock_t*);


/**
 * Trata de enviar todo el mensaje, aunque este sea muy grande.
 * Deja en len la cantidad de bytes NO enviados
 * Devuelve 0 en caso de exito, o -1 si falla
 */
int32_t send_bytes(sock_t*, void*, uint32_t*);


/**
 * Trata de recibir todo el mensaje, aunque este se envie en varias rafagas.
 * Deja en len la cantidad de bytes NO recibidos
 * Devuelve 0 en caso de exito; o -1 si falla
 */
int32_t receive_bytes(sock_t*, void*, uint32_t);

#endif /* SOCKETS_H_ */
