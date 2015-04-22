/*
 * Funciones.h
 *
 *  Created on: 19/04/2014
 *      Author: federico
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <commons/log.h>
#include <parser/metadata_program.h>
#include <sys/wait.h>
#include <semaphore.h>


extern sem_t sem_pcp;
extern sem_t sem_plp;
extern u_int32_t retardo;
extern int socketUmv;
extern int listenningSocket;
extern u_int32_t gradoMult;
extern u_int32_t gradoMultiprog;
extern t_dictionary* dicSemaforos;
extern t_dictionary* dicVarCompartidas;
extern t_dictionary* dicDispositivos;
extern t_queue* colaNuevos;
extern t_list* colaListos;
extern t_queue* colaBlock;
extern t_list* colaExec;
extern t_queue* colaExit;
extern t_log* logger;

typedef struct
{
	unsigned char code;
	size_t size;
} __attribute__((packed)) socket_header  ;

typedef struct
{
	int tiempo; 	//tiempo de retardo
	t_queue* bloqueados; //bloqueados por cada dispositivo
	sem_t semaforo;
	pthread_mutex_t mutex;
	pthread_t hiloDispositivo;

} __attribute__((packed)) IO;

typedef struct
{
	int valor; 	//valor de sem
	t_queue* bloqSem; //bloqueados por semaforos
} __attribute__((packed)) t_semaforos;

typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
	char** id_IO;
	char** IO_time;
	char** semaforos;
	char** valorSemaforo;
	int multiprog;
	char* puerto_prog;
	char* puerto_cpu;
	int quantum;
	int retardo;
	char* ipUMV;
	char* puerto_umv;
	char** varGlobales;
	int stackSize;
}Kernel;
typedef struct estructura_pcb					//estructura que creo con los datos que devuelve el parser
{
	u_int32_t processID;
	u_int32_t progCounter;
	u_int32_t stackSegment;
	u_int32_t codSegment;
	u_int32_t etiq_Segment;
	u_int32_t cursor_stack;
	u_int32_t cantVariables;
	u_int32_t codIndex;
	u_int32_t tam_indice_etiquetas;
	u_int32_t peso;
	u_int32_t socketPrograma;
	u_int32_t prioridad;
}PCB;

typedef struct retornoUMV
{
	u_int32_t punteroCodigo;
	u_int32_t punteroEtiq;
	u_int32_t codIndex;			//este seria el indice de codigo
	u_int32_t stack;
} __attribute__((packed)) estructuraUMV;

typedef struct
{
	u_int32_t size;
	char* codigo;
}paquetePrograma;
typedef struct
{
	int unidadesDeTiempo;
}nodoDispositivo;

void mostrar_config(Kernel*);
Kernel* crear_estructura_config(char*);
int calculo_peso_PCB(PCB*, t_medatada_program*);
PCB* crear_estructura_PCB( t_medatada_program*, estructuraUMV*, u_int32_t);
void mostrar_estructura_PCB(void*);
t_queue* crear_cola_new();
void encolar_proceso_nuevo( PCB*);
void encolar_proceso_listo_creado();
void mostrar_cola_nuevos();
void mostrar_cola_listos();
void mostrar_cola_block();
void mostrar_cola_exec();
void mostrar_cola_exit();
estructuraUMV* reservar_segmentos_umv(char*);
char* serializar_datos_umv(t_medatada_program*,char*);
void obtenerTokensSemaforos(char**, char**);
void obtenerTokensDispositivos(char**, char**);
void obtenerTokensCompartidas(char**);
void liberarNodo(char*);
void aviso_programa();
char* serializoFinPrograma();
estructuraUMV* deserializarUMV();
t_queue* crear_cola_exit();
t_list* crear_cola_ready();
void dispose_package(char **);
int obtengoSocketCliente();
int obtenerSocketServidor(char*);
int crearSocketCliente(char*, char*);
void atender_cliente(int, int);
void chequeo_multiprog();
void *hilo_rec_res();
void meterEnExit(PCB*);
void *thread_IO(void*);
PCB* sacoPcbDeLaCola(u_int32_t);


#endif /* FUNCIONES_H_ */
