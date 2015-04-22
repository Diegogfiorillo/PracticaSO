/*
 * Funciones.c
 *
 *  Created on: 19/04/2014
 *      Author: federico
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "FuncionesPLP.h"
#include "FuncionesPCP.h"
#include <pthread.h>


pthread_mutex_t Multiprogramacion = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MutexListos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MutexBlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MutexExec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MutexExit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MutexNuevos = PTHREAD_MUTEX_INITIALIZER;
fd_set socket_copy, socketLectura;
u_int32_t id=50;

Kernel* crear_estructura_config(char* path) {
	t_config* archConfig = config_create(path);
	Kernel* config = malloc(sizeof(Kernel));
	config->IO_time = config_get_array_value(archConfig, "HIO");
	config->id_IO = config_get_array_value(archConfig, "ID_HIO");
	config->multiprog = config_get_int_value(archConfig, "MULTIPROGRAMACION");
	config->puerto_cpu = config_get_string_value(archConfig, "PUERTO_CPU");
	config->puerto_prog = config_get_string_value(archConfig, "PUERTO_PROG");
	config->quantum = config_get_int_value(archConfig, "QUANTUM");
	config->retardo = config_get_int_value(archConfig, "RETARDO");
	config->semaforos = config_get_array_value(archConfig, "SEMAFOROS");
	config->valorSemaforo = config_get_array_value(archConfig,
			"VALOR_SEMAFORO");
	config->ipUMV = config_get_string_value(archConfig, "IP_UMV");
	config->puerto_umv = config_get_string_value(archConfig, "PUERTO_UMV");
	config->varGlobales = config_get_array_value(archConfig, "COMPARTIDAS");
	config->stackSize= config_get_int_value(archConfig, "TAMANIO_STACK");
	return config;
}


void obtenerTokensSemaforos(char** arrSemaforos, char** arrValorSemaforos)	//obtiene los tokens y los guarda en una lista
{
	t_semaforos* semaforos;

	dicSemaforos = dictionary_create();

	int i;

	for(i = 0; arrSemaforos[i] != NULL && arrValorSemaforos[i] != NULL; i++)
	{
		semaforos = malloc(sizeof(t_semaforos));
		semaforos->valor = atoi(arrValorSemaforos[i]);
		semaforos->bloqSem = queue_create();

		dictionary_put(dicSemaforos, arrSemaforos[i], semaforos);
	}

	string_iterate_lines(arrSemaforos, liberarNodo);
	string_iterate_lines(arrValorSemaforos, liberarNodo);
	free(arrSemaforos);
	free(arrValorSemaforos);

}

void obtenerTokensCompartidas(char** arrVarGlobales)	//obtiene los tokens y los guarda en una lista
{

	dicVarCompartidas = dictionary_create();

		int i;

		for(i = 0; arrVarGlobales[i] != NULL; i++)
		{
			int32_t *valor = malloc(sizeof(int32_t));
			*valor = 0;
			dictionary_put(dicVarCompartidas, arrVarGlobales[i], valor);
		}

		string_iterate_lines(arrVarGlobales, liberarNodo);
		free(arrVarGlobales);
}

void obtenerTokensDispositivos(char** arrDispositivos, char** arrValorDispositivos)	//obtiene los tokens y los guarda en una lista
{
	IO* dispositivos;
	dicDispositivos = dictionary_create();

		int i;

		for(i = 0; arrDispositivos[i] != NULL; i++)
		{
			dispositivos = malloc(sizeof(IO));
			pthread_mutex_init(&dispositivos->mutex, NULL);
			sem_init(&dispositivos->semaforo, 0, 0);
			dispositivos->tiempo = atoi(arrValorDispositivos[i]);
			dispositivos->bloqueados = queue_create();
			pthread_create(&dispositivos->hiloDispositivo, NULL, thread_IO, (void*)dispositivos);
			dictionary_put(dicDispositivos, arrDispositivos[i], dispositivos);
		}
		string_iterate_lines(arrDispositivos, liberarNodo);
		string_iterate_lines(arrValorDispositivos, liberarNodo);
		free(arrDispositivos);
		free(arrValorDispositivos);

}

void liberarNodo(char* cad)
{
	free(cad);
}


int calculo_peso_PCB(PCB* pcb, t_medatada_program* datosParser) {
	pcb->peso = 5 * datosParser->cantidad_de_etiquetas
			+ 3 * datosParser->cantidad_de_funciones
			+ datosParser->instrucciones_size;
	return pcb->peso;
}

PCB* crear_estructura_PCB( t_medatada_program* datos,
		estructuraUMV* segmentos_reservados_umv, u_int32_t SocketPrograma) {
	PCB* estructuraPCBNuevo = malloc(sizeof(PCB));
	estructuraPCBNuevo->processID = id;
	estructuraPCBNuevo->progCounter = datos->instruccion_inicio;
	estructuraPCBNuevo->stackSegment = segmentos_reservados_umv->stack;
	estructuraPCBNuevo->codSegment = segmentos_reservados_umv->punteroCodigo;
	estructuraPCBNuevo->etiq_Segment = segmentos_reservados_umv->punteroEtiq;
	estructuraPCBNuevo->cursor_stack = segmentos_reservados_umv->stack;
	estructuraPCBNuevo->cantVariables = 0;
	estructuraPCBNuevo->codIndex = segmentos_reservados_umv->codIndex;
	estructuraPCBNuevo->tam_indice_etiquetas = datos->etiquetas_size;
	estructuraPCBNuevo->peso = calculo_peso_PCB(estructuraPCBNuevo, datos);
	estructuraPCBNuevo->socketPrograma= SocketPrograma;
	estructuraPCBNuevo->prioridad=3;
	id++;
	return estructuraPCBNuevo;
}

void mostrar_estructura_PCB_Nuevo(void* pcb)//funciona de closure en el list_iterate
{
	printf("Proceso: %d\n", ((PCB*) pcb)->processID);
	printf("Peso del proceso %d: %d\n", ((PCB*) pcb)->processID,
			((PCB*) pcb)->peso);
	printf(" \n");
}

void mostrar_estructura_PCB(void* pcb)	//funciona de closure en el list_iterate
{
	printf("Proceso: %d\n", ((PCB*) pcb)->processID);
	printf("Prog Counter:%d\n", ((PCB*) pcb)->progCounter);
	printf("Socket programa: %d\n",((PCB*) pcb)->socketPrograma) ;
	printf(" \n");
}

t_queue* crear_cola_new() {
	t_queue* colaProcesoNuevo = queue_create();
	return colaProcesoNuevo;
}

t_list* crear_cola_ready() {
	t_list* colaProcesoListo = list_create();
	return colaProcesoListo;
}

void encolar_proceso_nuevo(PCB* pcbNuevo) {
	pthread_mutex_lock(&MutexNuevos);
	queue_push(colaNuevos, (PCB*) pcbNuevo);
	pthread_mutex_unlock(&MutexNuevos);
}

void mostrar_cola_nuevos() {
	printf("Estado de la cola de procesos en espera para pasar a Listos...\n");
	printf(" \n");
	pthread_mutex_lock(&MutexNuevos);
	list_iterate(colaNuevos->elements, mostrar_estructura_PCB_Nuevo);
	pthread_mutex_unlock(&MutexNuevos);
}

void mostrar_cola_listos() {
	printf("Estado de la cola de procesos en espera para ser ejecutados...\n");
	printf(" \n");
	pthread_mutex_lock(&MutexListos);
	list_iterate(colaListos, mostrar_estructura_PCB);
	pthread_mutex_unlock(&MutexListos);
}
void mostrar_cola_block() {
	printf("Estado de la cola de procesos bloqueados...\n");
	printf(" \n");
	pthread_mutex_lock(&MutexBlock);
	list_iterate(colaBlock->elements, mostrar_estructura_PCB);
	pthread_mutex_unlock(&MutexBlock);
}
void mostrar_cola_exec() {
	printf("Estado de la cola de procesos ejecutandose...\n");
	printf(" \n");
	pthread_mutex_lock(&MutexExec);
	list_iterate(colaExec, mostrar_estructura_PCB);
	pthread_mutex_unlock(&MutexExec);
}

void mostrar_cola_exit() {
	printf("Estado de la cola de procesos finalizados...\n");
	printf(" \n");
	pthread_mutex_lock(&MutexExit);
	list_iterate(colaExit->elements, mostrar_estructura_PCB);
	pthread_mutex_unlock(&MutexExit);
}
void encolar_proceso_listo_creado() {

	bool OrdenoPorPeso(void *uno, void *dos)
		{
			return ((PCB*) uno)->peso < ((PCB*) dos)->peso;
		}
	list_sort(colaNuevos->elements, OrdenoPorPeso);


	pthread_mutex_lock(&MutexListos);
	list_add(colaListos, queue_pop(colaNuevos));
	pthread_mutex_unlock(&MutexListos);
	printf("Pero ya encole te dije\n");
	pthread_mutex_lock(&Multiprogramacion);
	gradoMult++;
	pthread_mutex_unlock(&Multiprogramacion);


}

estructuraUMV* reservar_segmentos_umv(char* paqueteSerializado) {

	estructuraUMV* segmentos = malloc(sizeof(estructuraUMV));
	int recibio = recv(socketUmv, segmentos, sizeof(estructuraUMV), 0);
	if(recibio==-1)
	{
		log_error(logger, "No se pudo recibir el paquete");
		segmentos->codIndex=-1;
		segmentos->punteroCodigo=-1;
		segmentos->punteroEtiq=-1;
		segmentos->stack=-1;
		return segmentos;
	}
	return segmentos;

}



char* serializar_datos_umv(t_medatada_program *paquete, char* codigo) {

	char *paqueteSerializado = malloc(
			4 * sizeof(u_int32_t) + strlen(codigo)
					+ 2 * sizeof(u_int32_t) * paquete->instrucciones_size
					+ paquete->etiquetas_size);

	int offset = 0;
	int size_to_send;
	int tam = strlen(codigo);

	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, &(id), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, &tam, size_to_send);
	offset += size_to_send;

	size_to_send = tam;
	memcpy(paqueteSerializado + offset, codigo, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, &(paquete->instrucciones_size),
			size_to_send);
	offset += size_to_send;

	size_to_send = 2 * sizeof(u_int32_t) * paquete->instrucciones_size;
	memcpy(paqueteSerializado + offset, paquete->instrucciones_serializado,
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, &(paquete->etiquetas_size),
			size_to_send);
	offset += size_to_send;

	size_to_send = paquete->etiquetas_size;
	memcpy(paqueteSerializado + offset, paquete->etiquetas, size_to_send);
	offset += size_to_send;

	int envio = send(socketUmv, paqueteSerializado, offset, 0);
	return paqueteSerializado;
}

void aviso_programa(int programaSocket) {

	u_int32_t enviar;
	u_int32_t digito=5;
	enviar = send(programaSocket, &digito, sizeof(u_int32_t),0);
		if (enviar == -1) {
			printf("Error al enviar el aviso \n");
		}
	enviarEnd(programaSocket);
	close(programaSocket);
}

char* serializoFinPrograma()
{
	u_int32_t digito=1;
	char* noHayMemoria="No hay espacio suficiente en la UMV";
	int tam= strlen(noHayMemoria);
	char* serializado = malloc(2*sizeof(u_int32_t)+tam);
	u_int32_t size_to_send;
	u_int32_t offset;

	size_to_send =  sizeof(u_int32_t);
	memcpy(serializado, &digito, size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(serializado, &tam, size_to_send);
	offset += size_to_send;

	size_to_send = tam;
	memcpy(serializado+offset,noHayMemoria,tam);

	return serializado;
}

t_queue* crear_cola_exit() {
	t_queue* colaProcesoTerminado = queue_create();
	return colaProcesoTerminado;
}

void *thread_IO(void* parametro)
{
	IO* nodo = (IO*)parametro;
	while(1)
	{
		sem_wait(&(nodo->semaforo));
		if( !(queue_is_empty(nodo->bloqueados)) )
		{
			t_disp* pedido=queue_pop(nodo->bloqueados);
			usleep((nodo->tiempo)*(pedido->tiempo));
			bool find_by_id(void* parametro)
				{
					PCB* pcb = (PCB*) parametro;
					return (pcb->processID)==(pedido->idP);
				}
			if(list_any_satisfy(colaBlock->elements, find_by_id)){
			PCB* pcb=sacar_proceso_block(pedido->idP);
			mostrar_cola_block();
			pthread_mutex_lock(&MutexListos);
			list_add(colaListos, pcb);
			pthread_mutex_unlock(&MutexListos);
			mostrar_cola_listos();
			sem_post(&sem_pcp);
			}
		}
	}
	return 0;
}


void dispose_package(char **package) {
	free(*package);
}

int obtenerSocketServidor(char* puertoCliente) {

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, puertoCliente, &hints, &serverInfo);

	int socketEscucha;
	socketEscucha = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	if (socketEscucha == -1 && socketEscucha == 0) {
		printf("Fallo en el socket, se espera 1 minuto y se vuelve a intentar");
		sleep(6);
		socketEscucha = socket(serverInfo->ai_family,
				serverInfo->ai_socktype, serverInfo->ai_protocol);

	}
	bind(socketEscucha, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	return socketEscucha;
}

void *hilo_rec_res() {

	printf("Este es el hilo receptor\n");
	int optval = 1;
	ioctl(listenningSocket, FIONBIO, &optval);

	int socketCliente = obtengoSocketCliente();
	if (socketCliente == -1) {
			log_info(logger, "concluye el hilo receptor de programas");
	}
	return 0;
}

	int obtengoSocketCliente() {

		int i, socketCliente;
		bool finalizo= false;
		struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
		socklen_t addrlen = sizeof(addr);


		int maxfd = listenningSocket;
		FD_ZERO(&socketLectura);
		FD_ZERO(&socket_copy);
		FD_SET(listenningSocket, &socketLectura);

		do {
			memcpy(&socket_copy, &socketLectura, sizeof(fd_set));

			int variable = select(maxfd + 1, &socket_copy, NULL, NULL, NULL);

			if (variable) {
				//si es nueva conexion, agrego a la lista
				for (i = 0; i < maxfd + 1; i++) {
					if (FD_ISSET(i, &socket_copy)) {

						if (i == listenningSocket) {
							do {
								socketCliente = accept(listenningSocket,
										(struct sockaddr *) &addr, &addrlen);
								if (socketCliente < 0) {
									if (errno != EWOULDBLOCK) {
										log_error(logger,
												"Error al aceptar conexion entrante");
										finalizo = true;
															   }
									break;
														}
								log_info(logger, "Se acepto un nuevo programa");
								if (maxfd < socketCliente)
									maxfd = socketCliente;
								FD_SET(socketCliente, &socketLectura);

							} while (socketCliente!=-1);
						} else {
							int longitud;
							int datosRecibidos = recv(i,&(longitud),sizeof(u_int32_t),0);
							if(longitud==1 ){
								FD_CLR(i, &socketLectura);
								FD_CLR(i, &socket_copy);
								close(i);
							}
							if(!datosRecibidos) abort();
							if (longitud > 1) atender_cliente(i,longitud);
							}			//else
					} //ISSET
				}			//for

			}			//if
		} while (finalizo == false);
		printf("SE ME CERRO EL SELECT \n\n\n\n\n\n\n\n\n\n\n");
		for (i = 0; i <= maxfd; ++i) {
				if (FD_ISSET(i, &socketLectura)){
					close(i);
				FD_CLR(i, &socketLectura);
			}
		}
//		sem_destroy(&sem_prog);
		return -1;
	}

	int crearSocketCliente(char* ipServidor, char* puertoServidor) {
		struct addrinfo hints;
		struct addrinfo *ServerInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
		hints.ai_socktype = SOCK_STREAM;// Indica que usaremos el protocolo TCP

		getaddrinfo(ipServidor, puertoServidor, &hints, &ServerInfo);// Carga en serverInfo los datos de la conexion

		int serverSocket;
		serverSocket = socket(ServerInfo->ai_family, ServerInfo->ai_socktype,
				ServerInfo->ai_protocol);
		if (serverSocket == -1) {
			printf(
					"Fallo en el socket, se espera 1 minuto y se vuelve a intentar");
			sleep(6);
			serverSocket = socket(ServerInfo->ai_family,
					ServerInfo->ai_socktype, ServerInfo->ai_protocol);
		}
		connect(serverSocket, ServerInfo->ai_addr, ServerInfo->ai_addrlen);
		freeaddrinfo(ServerInfo);	// No lo necesitamos mas

		return serverSocket;
	}

	void chequeo_multiprog() {

		if (gradoMult < gradoMultiprog && !queue_is_empty(colaNuevos)) {
				log_info(logger,
						"Se agrega a la cola de listos un nuevo pcb");
				printf("Procedo a encolar\n");
				printf("Pete\n");
				encolar_proceso_listo_creado();
				printf("Hola, encole\n");
				puedoPasarACpu();
		}
	}

	void atender_cliente(int socketCliente,int size) {
		char* fin="fin";

		printf("Me llega un size: %d\n", size);
//		printf("El codigo: %s\n", codigo);
		if(size==3){
			log_info(logger,"Se dio el baja el programa");
			PCB* unPcb= sacoPcbDeLaCola(socketCliente);
			printf("%d\n",unPcb->processID);
			if(unPcb != NULL){
				u_int32_t digito = 3;
				int valor=destruir_segmentos(unPcb);
				if(valor>0)printf("Destrui bien\n");
				send(socketCliente,&digito, sizeof(u_int32_t),0);
			}

			return;
		}else if((size!=3)&&(size!=1)){
		char* codigo = malloc(size);
		int status = recv(socketCliente,codigo,size,0);
		t_medatada_program* datosPreParser = metadata_desde_literal(codigo); //preprocesador del parser
		char* datosSerializados = serializar_datos_umv(datosPreParser,codigo);
		estructuraUMV* segmentos_reservados_umv = reservar_segmentos_umv(
				datosSerializados);

		if ((segmentos_reservados_umv->punteroCodigo!=-1) && (segmentos_reservados_umv->codIndex!=-1) &&(segmentos_reservados_umv->punteroEtiq!=-1) &&(segmentos_reservados_umv->stack!=-1)) {  //Todo arreglar condicion de reserva
			log_info(logger, "Segmentos del proceso %d reservados", id);
			PCB* pcbNuevo = crear_estructura_PCB(datosPreParser,
					segmentos_reservados_umv, socketCliente);			//creo el PCB
			log_info(logger, "Creo el PCB\n");
			encolar_proceso_nuevo(pcbNuevo);	//encolo
			mostrar_cola_nuevos();
			chequeo_multiprog();
			} else {
			log_error(logger,
					"No hay espacio en la UMV, segmentos no reservados");
			aviso_programa(socketCliente);
		}
		free(datosPreParser);
		free(codigo);
		return;
	}
	}
void meterEnExit( PCB* pcb){
	pthread_mutex_lock(&MutexExit);
	queue_push(colaExit, pcb);
	pthread_mutex_unlock(&MutexExit);
	destruir_segmentos(pcb);
	pthread_mutex_lock(&Multiprogramacion);
	gradoMult--;
	pthread_mutex_unlock(&Multiprogramacion);
	chequeo_multiprog();
}
PCB* sacoPcbDeLaCola(u_int32_t socketCliente){
	bool find_by_id(void* parametro)
	{
		PCB* pcb = (PCB*) parametro;
		return pcb->socketPrograma==socketCliente;
	}

	pthread_mutex_lock(&MutexNuevos);
	if(!list_is_empty(colaNuevos->elements)){
		PCB* pcb=list_remove_by_condition(colaNuevos->elements, find_by_id);
		if(pcb != NULL) return pcb;
	}
	pthread_mutex_unlock(&MutexNuevos);

	pthread_mutex_lock(&MutexListos);
	if(!list_is_empty(colaListos)){
	PCB* pcb2=list_remove_by_condition(colaListos, find_by_id);
	if(pcb2 != NULL) return pcb2;
	}
	pthread_mutex_unlock(&MutexListos);

	pthread_mutex_lock(&MutexBlock);
	if(!list_is_empty(colaBlock->elements)){
	PCB* pcb3=list_remove_by_condition(colaBlock->elements, find_by_id);
	if(pcb3 != NULL)return pcb3;
	}
	pthread_mutex_unlock(&MutexBlock);

	pthread_mutex_lock(&MutexExec);
	if(!list_is_empty(colaExec)){
	PCB* pcb4=list_remove_by_condition(colaExec, find_by_id);
	if(pcb4 != NULL) return pcb4;
	}
	pthread_mutex_unlock(&MutexExec);
	return NULL;
}
