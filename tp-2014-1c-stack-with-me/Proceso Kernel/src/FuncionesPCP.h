/*
 * FuncionesPCP.h
 *
 *  Created on: 28/04/2014
 *      Author: federico
 */
 #ifndef FUNCIONESPCP_H_
 #define FUNCIONESPCP_H_
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include "FuncionesPLP.h"
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

extern fd_set socket_copy;
extern fd_set socketLectura;
 extern t_queue* colaNuevos;
 extern t_list* colaListos;
 extern t_queue* colaBlock;
 extern t_list* colaExec;
 extern t_queue* colaExit;
 extern int  socketUmv;
 extern int socketCpu;
 extern u_int32_t quantum;
 extern pthread_mutex_t MutexBlock;
 extern pthread_mutex_t MutexListos;
 extern pthread_mutex_t MutexExec;
 extern t_log* logger;
extern sem_t sem_pcp;


typedef struct{
int tamanio;
char* var;
int valor;
int socketPrograma;
}t_Variable;

typedef struct{
int tamanio;
char* dispositivo;
int tiempo;
PCB* pcb;
}t_Disp;

typedef struct{
int socket;
}t_CPU;

typedef struct
{
	int idP;
	int tiempo;
}t_disp;

typedef struct{
int tamanio;
char* sem;
PCB* pcb;
}t_Semaforo;

typedef struct{
int tamanio;
char* sem;
}t_SemaforoSig;


typedef struct{
	int tamanio;
	char* texto;
	int socketPrograma;
}t_texto;

typedef struct{
	int tamanio;
	char* var;
	int valor;
}t_compartida;

typedef struct{
	int tamanio;
	char* var;
	}t_pedidoCompartida;

t_list* crear_cola_exec();
t_queue* crear_cola_block();
void encolar_proceso_block( PCB*);
int encolar_proceso_listo();
char* serializarPCB(PCB* );
int atiendoCpus();
void atender_cpu(int, int);
char* recibir_de_CPU(int);
char* serializoTexto(t_texto*);
char* serializoVariable(t_Variable*);
t_texto* deserializoTexto(int);
t_Variable* deserializoVariable(int);
t_compartida* deserializoVariableCompartida(int);
t_Semaforo* deserializoSemaforo(int);
void asignoVarCompartida(char*, int);
void buscar_y_SacarDeExec(int);
void encolar_en_dispositivo(char*, int, int);
PCB* sacarDeListos();
void meterEnExec( PCB*);
int envioCpu (PCB*, int);
int destruir_segmentos(PCB*);
PCB* deserializarPCB(int);
char* serializarDestruir(u_int32_t);
void wait_sem(int, t_Semaforo*);
void signal_sem(int, t_SemaforoSig*);
PCB* sacar_proceso_block(int);
char* serializoSegFault();
t_Disp* deserializoDispositivo(int);
t_pedidoCompartida* deserializoPedidoCompartida(int);
t_SemaforoSig* deserializoSemaforoSignal(int);
int obtengoValor(char*);
int enviar_digito_imprimir(int,int);
void enviarEnd(int );
void* hilo_disponibilidad();
void mostrar_contenido(void*);
void puedoPasarACpu();
#endif /* FUNCIONESPCP_H_ */
