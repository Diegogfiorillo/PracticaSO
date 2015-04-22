#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <parser/parser.h>
#include "ejecucion.h"
#include "kernel.h"
#include "funcionesCpu.h"


extern PCB* pcbAUtilizar;
extern int quantum;
extern int quantumRestante;
int codigoFinalizacion;
t_dictionary *diccionario;
extern t_log* logger;
extern int finalizarPorSenal;
extern int clienteKernelCPU;
extern int clienteUMVCPU;
extern AnSISOP_funciones* funcionesPrimitivas;
extern AnSISOP_kernel* funcionesPrimitivasKernel;


void *ejecutarCPU (){
	diccionario = dictionary_create();
	while(!finalizarPorSenal){

		printf("Empezamos a ejecutar\n");
		PCB* pcbVacio = crearPcbVacio();
		char* pcbSerializado_paraEnviar = serializarPCB_con_digito(1,pcbVacio);
		int tam=13 * sizeof(u_int32_t);
		if(send(clienteKernelCPU,pcbSerializado_paraEnviar,tam,0)==-1){
			log_error(logger, "Error al enviar PCB");
			return 0;
		}
		free(pcbSerializado_paraEnviar);
		recv(clienteKernelCPU, &quantum, sizeof(u_int32_t),0);
		quantumRestante = quantum;
		pcbAUtilizar = malloc(12*sizeof(u_int32_t));
		int cant = recibir_y_deserializar_PCB();
		printf("cursor stack:%d\n", pcbAUtilizar->cursor_stack);
		printf("stack segment:%d\n", pcbAUtilizar->stackSegment);
		codigoFinalizacion = 4;
		pcbAUtilizar->prioridad = 1;
		diccionario=regenerarDiccionario(1, pcbAUtilizar, diccionario);

		while(quantumRestante>0){

			est_linea informacionLinea = obtenerLinea();
			char* informacionLineaSerializada = serializarInfoLinea(pcbAUtilizar->codSegment , informacionLinea.offset, informacionLinea.longitud, pcbAUtilizar->processID);
			int tam = 5*sizeof(u_int32_t);
			if(send(clienteUMVCPU,informacionLineaSerializada,tam,0)==-1){
				log_error(logger, "Error al enviar datos de la linea");
			}

			free(informacionLineaSerializada);
			u_int32_t verificacion;
			recv(clienteUMVCPU,&verificacion,sizeof(u_int32_t), 0);
			if(verificacion==0){
				quantumRestante=0;
				codigoFinalizacion=12;

			}else if(verificacion==1){
			char* linea = malloc((informacionLinea.longitud)+1);

			if(recv(clienteUMVCPU, linea, informacionLinea.longitud , 0)==-1){
				log_error(logger, "Error al recibir la linea");
			}else{
				linea[informacionLinea.longitud]='\0';
				printf("La linea que llego es: %s", linea);
				printf("\n");
				(pcbAUtilizar->progCounter)+=1;
				printf("Voy ejecutar la linea\n");
				analizadorLinea(linea, funcionesPrimitivas, funcionesPrimitivasKernel);
				printf("Linea analizada\n");
			}
			free(linea);

			printf("la siguiente linea a ejecutar es: %d", pcbAUtilizar->progCounter);
			quantumRestante-=1;
			log_info(logger, "Termino un quantum");

			}

		}

		log_info(logger, "Termino el ciclo de quantums");
		if(codigoFinalizacion!=3 && codigoFinalizacion!=8){
		pcbSerializado_paraEnviar = serializarPCB_con_digito(codigoFinalizacion,pcbAUtilizar);
		int tamanio=13 * sizeof(u_int32_t);
		if(send(clienteKernelCPU,pcbSerializado_paraEnviar,tamanio,0)==-1){
			log_error(logger, "Error al enviar PCB");
			return 0;
								 }
		}
	}
	log_info(logger, "Finalizar CPU por señal SIGUSR1");
	return 0;

}
