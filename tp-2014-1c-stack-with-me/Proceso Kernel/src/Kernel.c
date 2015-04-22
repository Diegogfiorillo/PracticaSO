/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <commons/log.h>
#include "FuncionesPCP.h"
#include "FuncionesPLP.h"

#define FONDONEGRO "\x1B[40m"
#define AMARILLO "\x1B[33m"
#define BLANCO "\x1B[37m"
#define MAXDATASIZE 100
#define BACKLOG 10
u_int32_t gradoMult = 0; 		//seteo el grado de multiprogramacion actual
u_int32_t pos = 0; 			//posicion para sacar los semaforos de la lista
sem_t sem_plp;
sem_t sem_pcp;
sem_t mutex_nuevos;
sem_t mutex_exec;
sem_t mutex_exit;
sem_t mutex_listos;
sem_t mutex_block;
t_dictionary* dicSemaforos;
t_dictionary* dicVarCompartidas;
t_dictionary* dicDispositivos;
u_int32_t retardo=0;
u_int32_t gradoMultiprog=0;
int socketUmv;
int listenningSocket; //socket de atencion de programas
int socketCpu;
u_int32_t quantum=0;
t_queue* colaNuevos;
t_list* colaListos;
t_queue* colaBlock;
t_list* colaExec;
t_queue* colaExit;
t_log* logger;
t_queue* colaCpuDisponibles;



void *thread_plp() {




	log_info(logger,"Se crea el socket servidor para los Programas");
	listen(listenningSocket, BACKLOG);

	pthread_t hilo_recibo_reservo;
	if (pthread_create(&hilo_recibo_reservo, NULL, hilo_rec_res, NULL)) {
		log_error(logger,"Error al crear el hilo de recepcion de programas y reserva de segmentos");
		abort();
	}

	pthread_join(hilo_recibo_reservo,NULL);
	close(listenningSocket);
	log_info(logger,"Se da por concluido el PLP");
	return 0;

}

//crear hilo PCP

void *thread_pcp() {

	int optval = 1;
	ioctl(socketCpu, FIONBIO, &optval);
	listen(socketCpu, BACKLOG);
	int retorno= atiendoCpus();
	if(retorno==-1){
		close(socketCpu);
		log_info(logger,"Se da por concluido el PCP");
	}
	usleep(retardo);

	return 0;
	}

void limpiar(){
	sem_destroy(&sem_plp);
	sem_destroy(&mutex_block);
	sem_destroy(&mutex_exec);
	sem_destroy(&mutex_exit);
	sem_destroy(&mutex_nuevos);
	sem_destroy(&mutex_listos);
	queue_destroy_and_destroy_elements(colaNuevos, free);
	list_destroy_and_destroy_elements(colaListos,free);
	list_destroy_and_destroy_elements(colaExec,free);
	queue_destroy_and_destroy_elements(colaBlock, free);
	queue_destroy_and_destroy_elements(colaExit, free);
}


void *thread_umv(void* parametro) {
	Kernel* arch= (Kernel*)parametro;
	u_int32_t stackTam=arch->stackSize;
	socketUmv = crearSocketCliente(arch->ipUMV,arch->puerto_umv);

		if(socketUmv==-1)
			{
				log_error(logger,"El socket de la Umv no se pudo conectar");
			}

		socket_header handshake;

		handshake.size = sizeof(socket_header);
		handshake.code = 'k'; //Kernel

		if( send(socketUmv, &handshake, sizeof(socket_header), 0) <= 0 )
		{
			log_error(logger, "No se puedo enviar Handshake a la UMV");
		}
		send(socketUmv, &stackTam, sizeof(u_int32_t), 0);
  return 0 ;
}

void *thread_cpu(void* parametro) {
	Kernel* arch= (Kernel*)parametro;
	socketCpu = obtenerSocketServidor(arch->puerto_cpu);
	return 0;
}

void *thread_prog(void* parametro) {
	Kernel* arch= (Kernel*)parametro;
	listenningSocket = obtenerSocketServidor(arch->puerto_prog);
	return 0;
}
void finalProvocado(){
	log_error(logger, "Se dio de baja el Kernel por parte del usuario");
	limpiar();
	exit(1);
}

int main(void) {
	printf("%s", FONDONEGRO);
	printf("%s", AMARILLO);
	printf(
			"	      _^__                                      __^__\n            ( ___ )------------------------------------( ___ )   \n             | / |                                      | \\ |  \n             | / |    Bienvenidos al proceso Kernel     | \\ |   \n             |___|                                      |___| \n            (_____)------------------------------------(_____)\n");


	char* path = "ConfigKernel.conf";
	Kernel* arch = crear_estructura_config(path);
	retardo=arch->retardo;
	gradoMultiprog=arch->multiprog;
	quantum=arch->quantum;


	char* archLog = "mi_log_kernel";
	logger = log_create("ProcesoKernel.c",archLog,1,LOG_LEVEL_INFO);

	if( signal(SIGINT, finalProvocado) == SIG_ERR ) {
		log_error(logger, "No se pudo tratar la senial");
	}

	pthread_t conectUMV;
			if (pthread_create(&conectUMV, NULL, thread_umv, arch)) {
				log_error(logger,"Error al crear el hilo de conexion con umv");
				abort();
			}
	pthread_t conectCpu;
			if (pthread_create(&conectCpu, NULL, thread_cpu, arch)) {
			log_error(logger,"Error al crear el hilo de conexion con cpu");
			abort();
	}
	pthread_t conectProg;
			if (pthread_create(&conectProg, NULL, thread_prog, arch)) {
			log_error(logger,"Error al crear el hilo de conexion de programas");
			abort();
	}

	pthread_join(conectUMV, NULL);
	pthread_join(conectCpu, NULL);
	pthread_join(conectProg, NULL);

	//Inicializo semaforos
	int semCreado = sem_init(&sem_plp,1,0);
	if(semCreado==-1)
	{
		log_error(logger,"No pudo crearse el semaforo del PLP");
	}
	int semPCP = sem_init(&sem_pcp,1,0);
	if(semPCP==-1)
	{
			log_error(logger,"No pudo crearse el semaforo del PLP");
	}
	int semNuevo = sem_init(&mutex_nuevos,1,1);
	if(semNuevo==-1)
	{
		log_error(logger,"No pudo crearse el semaforo de la cola de Nuevo");
	}
	int semExec = sem_init(&mutex_exec,1,1);
	if(semExec==-1)
	{
		log_error(logger,"No pudo crearse el semaforo de la cola de Exec");
	}
	int semExit = sem_init(&mutex_exit,1,1);
	if(semExit==-1)
	{
		log_error(logger,"No pudo crearse el semaforo de la cola de Exit");
	}
	int semBlock = sem_init(&mutex_block,1,1);
	if(semBlock==-1)
	{
		log_error(logger,"No pudo crearse el semaforo de la cola de Block");
	}
	int semListos = sem_init(&mutex_listos,1,1);
	if(semListos==-1)
	{
		log_error(logger,"No pudo crearse el semaforo de la cola de Listos");
	}

	log_info(logger, "Creando listas de semaforos y dispositivos");
	obtenerTokensSemaforos(arch->semaforos,arch->valorSemaforo); //creo lista de semaforos
	log_info(logger, "Lista de semaforos creada");
	obtenerTokensCompartidas(arch->varGlobales); //creo lista de variables globales
	log_info(logger, "Lista de variables compartidas creada");
	obtenerTokensDispositivos(arch->id_IO, arch->IO_time); //creo lista de dispositivos de i/o
	log_info(logger, "Lista de dispositivos creada");


	colaNuevos = crear_cola_new();      //creo la cola NEW
	colaListos = crear_cola_ready();      //creo la cola Ready
	colaBlock = crear_cola_block();      //creo la cola block
	colaExec = crear_cola_exec();		 //creo la cola exec
	colaExit = crear_cola_exit();		//creo la cola exit
	colaCpuDisponibles= queue_create();


	usleep(retardo);

	pthread_t mythreadPLP;
		if (pthread_create(&mythreadPLP, NULL, thread_plp, NULL)) {
			log_error(logger,"Error al crear el hilo del Plp");
			abort();
		}
	log_info(logger,"Thread PLP creado con exito");

	pthread_t mythreadPCP;
	if (pthread_create(&mythreadPCP, NULL, thread_pcp, NULL)) {
		log_error(logger,"Error al crear el hilo del Pcp");
		abort();
	}
	log_info(logger,"Thread PCP creado con exito");

	pthread_join(mythreadPLP,NULL);
	pthread_join(mythreadPCP,NULL);

	free(arch);
	limpiar();
	usleep(retardo);
	usleep(retardo);
	printf(
			"    ╔═════════════════════════╗ \n    ║Saludos,Federico y Diego.║ \n    ╚═════════════════════════╝");
	log_info(logger,"Damos por terminado el Proceso Kernel");
	log_destroy(logger);
	return 0;
}
