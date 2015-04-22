
#include <stdlib.h>
#include <stdio.h>
#include "commons/collections/list.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include "funcionesConsola.h"
#include <string.h>
#include <pthread.h>

#include "funcionesUMV.h"
#include "gestionDeSegmentos.h"
#include "serials.h"
#include "funcionesCPU.h"

#define BACKLOG 5
extern int retardo;
int claveContinuidadKernel=1;
typedef struct
{
	unsigned char code;
	size_t size;
} __attribute__((packed)) socket_header  ;


void  *Kernel_function(void* parametro)
{
	hiloKernelyCPU* argKernel=(hiloKernelyCPU*)parametro;
	int socketKernel = crearSocketServidor(argKernel->puerto);
	listen(socketKernel,BACKLOG);
	struct sockaddr_in addr;                                                      //establezco conexion
	socklen_t addrlen=sizeof(addr);
	int socketCliente=accept(socketKernel,(struct sockaddr*) &addr,&addrlen);

	socket_header handshake;
	while( recv( socketCliente, &handshake, sizeof(socket_header), MSG_WAITALL) != sizeof(socket_header) )
	{
		log_error(argKernel->logger,"Error al recibir handshake. Se intentara nuevamente.\n");
		printf("Error al recibir handshake. Se intentara nuevamente.\n");
	}

	if (handshake.code!='k')
	{
		log_error(argKernel->logger, "No se pudo establecer la conexion con el Kernel");
	}
	if(handshake.code=='k')
	{
		recv(socketCliente, &tamanio_stack, sizeof(u_int32_t), 0);

		while(claveContinuidadKernel)
		{
		t_deKernelAUMV* paqueteDeserializado = recibir_paquete_kernel(socketCliente);
		log_info(argKernel->logger, "Recibiendo paquetes ... \n");

		printf("Codigo size: %d \n", paqueteDeserializado->codigo_size);
		printf("Codigo: %s \n", paqueteDeserializado->codigo);
		printf("Etiquetas: %s \n", paqueteDeserializado->etiquetas);
		printf("Etiquetas size: %d \n", paqueteDeserializado->indice_etiquetas);
		printf("Instrucciones: %s \n",paqueteDeserializado->instrucciones);
		printf("Instrucciones size: %d \n", paqueteDeserializado->instrucciones_size);
		printf("Id proc: %d \n", paqueteDeserializado->id_proceso);



		int tamanio_total = 0;
		tamanio_total = (paqueteDeserializado->codigo_size)+(paqueteDeserializado->indice_etiquetas)+(2*sizeof(u_int32_t)*(paqueteDeserializado->instrucciones_size))+tamanio_stack;
//		printf("El tamanio total: %d\n", tamanio_total);
//		printf("El codigo size: %d\n", paqueteDeserializado->codigo_size);
//		printf("el tamanio de instrucciones %d\n", (2*sizeof(u_int32_t)*(paqueteDeserializado->instrucciones_size)));
//		printf("El size instrucciones: %d\n", strlen(paqueteDeserializado->instrucciones));
//		printf("El tamanio etiquetas: %d\n", paqueteDeserializado->indice_etiquetas);
		printf("Disponible: %d \n", disponible);
		usleep(retardo);

		if(paqueteDeserializado->codigo_size==0  && paqueteDeserializado->indice_etiquetas==0 && paqueteDeserializado->instrucciones_size==0)
		{
			printf("Destruyo segmentos del proceso %d\n",paqueteDeserializado->id_proceso );
			destruirTodosLosSegmentos(paqueteDeserializado->id_proceso);
			log_info(argKernel->logger,"Se han borrado todos los segmentos del proceso");
			printf("Se han borrado los segmentos del proceso %d. \n ", paqueteDeserializado->id_proceso);
		}
		else
		{
			sem_wait(&sem_disponible);

			if (tamanio_total<= disponible)
			{
				sem_post(&sem_disponible);

				log_info(argKernel->logger, "Los segmentos solicitados entran en la memoria\n");
				int id_proc = paqueteDeserializado->id_proceso;
				printf("Llego el proceso: %d\n", id_proc);
				int direccion_codigo = crearSegmento(paqueteDeserializado->codigo_size,id_proc,paqueteDeserializado->codigo);
				printf("direccion codigo: %d \n", direccion_codigo);
				if(direccion_codigo==-1)
				{
					log_error(argKernel->logger, "Tamaño de segmento menor a cero. No se pudo crear el segmento.\n");
				}

				int direccion_etiquetas = crearSegmento(paqueteDeserializado->indice_etiquetas,id_proc,paqueteDeserializado->etiquetas);
				printf("direccion etiquetas: %d \n", direccion_etiquetas);
				if(direccion_etiquetas==-1)
				{
					log_error(argKernel->logger, "Tamaño de segmento menor a cero. No se pudo crear el segmento.\n");
				}

				int direccion_instrucciones = crearSegmento(8*paqueteDeserializado->instrucciones_size,id_proc,paqueteDeserializado->instrucciones);
				printf("direccion  instrucciones: %d \n", direccion_instrucciones);
				if(direccion_instrucciones==-1)
				{
					log_error(argKernel->logger, "Tamaño de segmento menor a cero. No se pudo crear el segmento.\n");
				}
				char* stackito=malloc(tamanio_stack);
				int direccion_stack = crearSegmento(tamanio_stack,id_proc,stackito);
				printf("direccion stack: %d \n", direccion_stack);
				if(direccion_stack==-1)
				{
					log_error(argKernel->logger, "Tamaño de segmento menor a cero. No se pudo crear el segmento.\n");
				}



				t_deUMVaKernel* paquete_a_enviar=malloc(4*sizeof(u_int32_t));
				paquete_a_enviar->pcodigo = direccion_codigo;
				paquete_a_enviar->pinstrucciones = direccion_instrucciones;
				paquete_a_enviar->petiquetas = direccion_etiquetas;
				paquete_a_enviar->pstack = direccion_stack;


				int envio;
				envio=send(socketCliente,paquete_a_enviar,4*sizeof(int),0);
				if(envio!=-1 && envio!=0)
				{
					log_info(argKernel->logger, "Paquete enviado con exito\n");
				}
				else
				{
					log_error(argKernel->logger, "No se pudo enviar el paquete\n");
				}
				free(paquete_a_enviar);

			}//del if tamanio_total<= disponible
			else
			{
				printf("No hay espacio suficiente para guardar los 4 segmentos\n");
				log_error(argKernel->logger, "Memory Overload. No se pueden crear los segmentos\n");
				log_info(argKernel->logger, "Notificar que no se pudieron crear los segmentos solicitados\n");
				sem_post(&sem_disponible);

				t_deUMVaKernel* paquete_a_enviar=malloc((sizeof(t_deUMVaKernel)));
				paquete_a_enviar->pcodigo = -1;
				paquete_a_enviar->pinstrucciones=-1;
				paquete_a_enviar->petiquetas = -1;
				paquete_a_enviar->pstack = -1;

				int envio;
				envio=send(socketCliente,paquete_a_enviar,4*sizeof(u_int32_t),0);

				if(envio!=-1 && envio!=0)
				{
					log_info(argKernel->logger, "Paquete enviado con exito \n");
				}
				else
				{
					log_error(argKernel->logger, "No se pudo enviar el paquete\n");
				}
				free(paquete_a_enviar);
			}
		}// de un else
		if(claveContinuidadKernel==-1)
		{
			claveContinuidadKernel=0;
		}
		free(paqueteDeserializado);
	}// del while
	}

	//cerramos la conexion
	close(socketCliente);
	close(socketKernel);
	log_info(argKernel->logger,"Conexion del Kernel finalizada \n");
	return 0;
}

void  *Console_function(void* parametro)
{
	hiloConsola* argConsola = (hiloConsola*) parametro;
	FILE* archivoDump;
	while(1)
	{
		int opcionElegida;

		printf("Seleccione, entre las siguientes, la operación que desea realizar.\n");
		printf("-Operacion (0) \n");
		printf("-Retardo(1)\n");
		printf("-Cambiar algoritmo (2)\n");
		printf("-Compactar(3)\n");
		printf("-Dump(4)\n");
		printf("-Cerrar la Consola (5)\n");
		scanf("%d", &opcionElegida);
		switch (opcionElegida)
		{
		case 0: {
			funcionOperacion(argConsola->logger);
			break;  //fin caso elige operacion
		}
		case 1: {
			funcionRetardo(argConsola->logger);
			break;  //fin caso elige retardo
		}
		case 2: {
			funcionCambiarAlgoritmo(argConsola->logger);
			break; //fin caso elige cambiar algoritmo
		}
		case 3: {
			funcionCompactar(argConsola->logger);
			break; //fin caso elige compactar
		}
		case 4: {
			funcionDump(archivoDump);
			break;
		}
		case 5:
		{
			log_info(argConsola->logger,"Finalizado el hilo de la Consola");
			return EXIT_SUCCESS;
		}
		default:
		{
			printf("El comando ingresado no corresponde a ninguna de las operaciones\n");
			break;
		}
		} //fin switch
	} //fin while
	log_info(argConsola->logger,"Finalizado el hilo de la Consola");
	return 0;
}


void *hiloCpu(void* parametro)
{
	hiloCadaCPU* args = (hiloCadaCPU*)parametro;
	u_int32_t disponibleStack = tamanio_stack;

	socket_header handshake;
	int recibeHandshake = recv (args->socketsIndividuales, &handshake, sizeof(socket_header),MSG_WAITALL);
	if (recibeHandshake==-1)
	{
		log_error(args->logger, "No se pudo establecer la conexion con la CPU");
		printf("No se pudo establecer la conexion con la CPU");
	}

	if(handshake.code=='c')
	{
		log_info(args->logger, "Se conecto con la cpu");

		int claveContinuidadCPUs=1;
		while(claveContinuidadCPUs)
		{
		printf("busco una peticion\n");
		u_int32_t clavePeticion = recibirCodigoPeticion(args->socketsIndividuales);
		printf("recibio de peticion un: %d\n", clavePeticion);
		if(clavePeticion==0)
		{
			claveContinuidadCPUs=0;
		}
		switch(clavePeticion)
		{
		case 1:
		{
			t_Dir_deCPUaUMV* direccionCPU= recibir_Direccion_CPU(args->socketsIndividuales);
			printf("direccion: %d\n", direccionCPU->direccionDelaProximaLineaAEjecutar);
			printf("offset: %d\n", direccionCPU->offset);
			printf("longitud: %d\n", direccionCPU->tamanio);
			if(direccionCPU==0)
			{
				log_info(args->logger,"No se puede recibir de Cpu");
			}
			else
			{
				usleep(retardo);

				char* lineaCodigo = buscarLineaDeCodigoPedidaPorLaCPU(direccionCPU->direccionDelaProximaLineaAEjecutar, direccionCPU->tamanio,direccionCPU->offset, args->socketsIndividuales, direccionCPU->idDeProceso);
				printf("linea: %s \n", lineaCodigo);
			}
			free(direccionCPU);
			break;
		}
		case 2:
		{
			t_Nombre_deCPUaUMV* paqueteNombre = recibir_Nombre_CPU(args->socketsIndividuales);
			printf("la variable es: %c\n", paqueteNombre->nombre_variable);
			printf("en donde lo guardo: %d\n", paqueteNombre->offset);
			printf("corrido en %d posiciones\n", paqueteNombre->direccion);
			if(paqueteNombre==0)
			{
				log_error(args->logger,"No se puede recibir de Cpu");
			}

			else
			{
				usleep(retardo);
				agregarVariablesAlStack(paqueteNombre->direccion,paqueteNombre->offset,paqueteNombre->nombre_variable,disponibleStack, args->socketsIndividuales, paqueteNombre->idDeProceso);
			}
			free(paqueteNombre);
			break;
		}
		case 3:
		{
			t_DirYValor_deCPUaUMV* paqueteDirYValor = recibir_DirYValor_CPU(args->socketsIndividuales);
			printf("la valor de la variable es: %d\n", paqueteDirYValor->modificaion);
			printf("en donde lo guardo: %d\n",paqueteDirYValor->direccion_variable );
			printf("corrido en %d posiciones\n", paqueteDirYValor->offset);
			if(paqueteDirYValor==0)
			{
				log_error(args->logger,"No se puede recibir de Cpu");
			}
			else
			{
				usleep(retardo);
				modificarVariablesEnElStack(paqueteDirYValor->direccion_variable,paqueteDirYValor->offset, paqueteDirYValor->modificaion, args->socketsIndividuales, paqueteDirYValor->idDeProceso);
			}
			free(paqueteDirYValor);
			break;
		}
		default:
			break;
		}//switch
		clavePeticion=0;
		printf("La continuidad esta dada por el digito: %d\n", claveContinuidadCPUs);
	}//while
	}
		return 0;
}





void *CPU_function(void* parametros)
{
	hiloKernelyCPU* argCPU = (hiloKernelyCPU*) parametros;

	int socketCPU = crearSocketServidor(argCPU->puerto);
	listen(socketCPU, BACKLOG);
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int aceptado = 1;
	while(aceptado)
{
		int socketCliente = accept(socketCPU, (struct sockaddr *) &addr, &addrlen);
		log_info(argCPU->logger, "Se obtuvo la conexion proveniente del CPU");

		if (socketCliente!= -1)
		{
			pthread_t hiloNcpu;

			char* archLogCadaCPU= "mi_log_CadaCPU";  //los logs los haria global
			t_log* logCadaCPU = log_create("logCadaCpu.c",archLogCadaCPU,1,LOG_LEVEL_INFO);

			hiloCadaCPU* argCadaCPU = malloc(sizeof(hiloCadaCPU));
			argCadaCPU->logger=logCadaCPU;
			argCadaCPU->socketsIndividuales=socketCliente;

			if (pthread_create(&hiloNcpu, NULL,hiloCpu,(void*)argCadaCPU))
{
				log_error(argCPU->logger,"Error al crear el hilo de cpu");
				abort();
			}

		}
		else
{
			aceptado=0;
		}

}


	//cerramos la conexion
	close(socketCPU);
	log_info(argCPU->logger,"Finalizado el hilo de Cpu");
	return 0;
}

