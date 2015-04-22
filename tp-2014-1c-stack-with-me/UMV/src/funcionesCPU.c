/*
 * funcionesCPU.c
 *
 *  Created on: 17/07/2014
 *      Author: utnso
 */

#include "funcionesConsola.h"
#include "gestionDeSegmentos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionesCPU.h"
#include "commons/collections/list.h"
extern u_int32_t tamanio_stack;

extern t_list* listMP;
extern sem_t sem_listMP;

//Funciones que se usan para la CPU
char* buscarLineaDeCodigoPedidaPorLaCPU(u_int32_t direccion, u_int32_t tamanio, u_int32_t offset, int socketCpu, u_int32_t idDeProceso)
{

	sem_wait(&sem_listMP);

	int longitudLista = list_size(listMP);
	int index=0;
	int valorMio=1;
	char* informacion=malloc(tamanio);
	while(index>-1 && index<longitudLista)
	{
		memP* seg=list_get(listMP,index);
		if(seg->dir_logica == direccion && seg->id_proceso==idDeProceso)
		{
			printf("El offset es: %d\n", offset);
			printf("Lo max es %d\n", seg->longitud);
			if(offset<=seg->longitud){
				memcpy(informacion,(seg->memFisica + offset),tamanio);
				index=-1;
			}else{
				valorMio=0;
				index=-1;
			}


		}
		else
		{
			index++;
		}
	}
	if (index==longitudLista)
		{
			printf("No existe la direccion de memoria \n");
			valorMio=0;
		}
	sem_post(&sem_listMP);
	if(valorMio==0){
		printf("Envio el digito en %d\n", valorMio);
		u_int32_t envio=0;
		send(socketCpu, &envio, sizeof(u_int32_t), 0);
	}else if(valorMio==1){
		printf("Envio el digito en %d y la info\n", 1);
	printf("la linea es: %s", informacion);
			u_int32_t envio=1;
			send(socketCpu, &envio, sizeof(u_int32_t), 0);

			send(socketCpu,informacion,tamanio,0);

	return informacion;

	}

}

//la siguiente funcion es para grabar variables en el stack
void agregarVariablesAlStack(u_int32_t direccion, u_int32_t offset, char variable, u_int32_t disponibleStack, int socketDeCpu, u_int32_t idDeProceso)
{
	sem_wait(&sem_listMP);

	u_int32_t usado=offset+sizeof(char)+sizeof(u_int32_t);
	if(usado<=tamanio_stack){



	int longitudLista = list_size(listMP);
	printf("longitud lista: %d\n", longitudLista);
	int index=0;
	while(index>-1 && index<longitudLista)
	{
		memP* seg=list_get(listMP,index);
		if(seg->dir_logica == direccion && seg->id_proceso==idDeProceso)
		{
			if(disponibleStack >= (sizeof(variable)+sizeof(u_int32_t)))
			{  //corroboramos si en el stack sigue habiendo lugar para una variable mas

				grabaEnFisica(&variable,seg->memFisica + offset ,(sizeof(char)));
				//si hay lugar procedemos a grabar en memoria la variable, en la posicion siguiente a todas las demas variables ya grabadas, recordemos que la cpu solo nos da la posicion fisca de la pila, y nosotros grabamos al final de todas las variables ya guardadas

				disponibleStack = disponibleStack - (sizeof(char)+sizeof(u_int32_t));
				//como grabamos la variable reducimos la longitud del stack con el tamano de la variable nueva que se grabo
				index=-1;
			}
			else
			{
				printf("No hay mas espacio en el stack \n");
			}
		}//del if coincidir direcciones
		else
		{
			index++;
		}
	}//del while

	if(index==longitudLista)
	{
		printf("No existe el segmento stack de este proceso \n");
	}


	u_int32_t digito=1;
	send(socketDeCpu, &digito, sizeof(u_int32_t), 0);



	}else{
		printf("Segmentation Fault \nEsta intentando escribir fuera de los limites asignados.\n");
		u_int32_t digito=0;
		send(socketDeCpu, &digito, sizeof(u_int32_t), 0);
	}


	sem_post(&sem_listMP);
}

void modificarVariablesEnElStack (u_int32_t direccion, u_int32_t offset, int modificacion,int socketDeCpu, u_int32_t idDeProceso)
{
	sem_wait(&sem_listMP);


	u_int32_t usado=offset+sizeof(u_int32_t);
	if(usado<=tamanio_stack){


	if(sizeof(modificacion) == sizeof(u_int32_t))
	{
		int longitudLista = list_size(listMP);
		int index=0;
		while(index>-1 && index<longitudLista)
		{
			memP* seg=list_get(listMP,index);
			if(seg->dir_logica == direccion && seg->id_proceso==idDeProceso)
			{
				printf("Estoy guardando en la posicion %d el valor %d\n", ((seg->dir_logica)+offset),modificacion);
				printf("Tecnicamente empieza en %p\n", seg->memFisica);
				printf("Que tiene esta variable... %s\n", seg->memFisica);
				printf("Tecnicamente grabo en %p\n", (seg->memFisica+offset));
				printf("Que tiene esta variable... %s\n", (seg->memFisica+offset));
//				memcpy(((seg->memFisica)+offset),&modificacion,sizeof(u_int32_t));
				grabaValorEnFisica(&modificacion,((seg->memFisica)+offset), sizeof(u_int32_t));
				int valor;
				memcpy(&valor,((seg->memFisica)+offset),sizeof(u_int32_t));
				printf("y pasa a tener... %d\n", valor);
				/*la CPU me pasa la direccion de donde comienza en la memoria fisica la variable, la cual
				 *esta representada por 5 Bytes, y los ultimos 4 el valor, por lo tanto, me situo en la memoria fisica
				 *esta en el lugar que nos envio y le sumo un bite del nombre que ocupa la variable para grabarlo
				 *esta en los otros 4 bytes que le siguen
				 *esta */
				index=-1;
			}
			else
			{
				index++;
			}
		}
		if(index==longitudLista)
		{
			printf("No existe la direccion de memoria \n");
		}
	}
	else
	{
		printf("Segmentation Fault \nEsta intentando escribir fuera de los limites asignados.\n");
	}
	//analizar que pasa si la direccion que nos manda es erronea y pisa informacion que no tiene nada que ver

	u_int32_t digito=1;
	send(socketDeCpu, &digito, sizeof(u_int32_t), 0);


		}else{
			printf("Segmentation Fault \nEsta intentando escribir fuera de los limites asignados.\n");
			u_int32_t digito=0;
			send(socketDeCpu, &digito, sizeof(u_int32_t), 0);
		}


		sem_post(&sem_listMP);
}

char buscoLaVariableChar (u_int32_t direccion, u_int32_t offset, u_int32_t idDeProceso)
{
	sem_wait(&sem_listMP);
	int longitudLista = list_size(listMP);
	int index=0;
	char paquete;

	while(index>-1 && index<longitudLista)
	// en el while buscamos en la memoria el stack, ya que la cpu nos pasa el puntero del mismo, segun el proceso en el que este
	{
		memP* seg=list_get(listMP,index);
		if(seg->dir_logica == direccion && seg->id_proceso==idDeProceso)
		{
			memcpy(&paquete,seg->memFisica+offset,sizeof(char));
			index=-1;
		}
		else
		{
			index++;
		}
	}

	if (longitudLista==index)
	{
		sem_post(&sem_listMP);
		printf("No existe el segmento stack de este proceso \n");
		return '0';
	}

	sem_post(&sem_listMP);
	return paquete;
}

int buscoElContenidoInt (u_int32_t direccion, u_int32_t offset, u_int32_t idDeProceso)
{
	sem_wait(&sem_listMP);
	int longitudLista = list_size(listMP);
	int devuelta;
	int index=0;
	while(index>-1 && index<longitudLista)
		// en el while buscamos en la memoria el stack, ya que la cpu nos pasa el puntero del mismo, segun el proceso en el que este
	{
		memP* seg=list_get(listMP,index);
		if(seg->dir_logica == direccion && seg->id_proceso==idDeProceso)
		{
			memcpy(&devuelta, seg->memFisica+offset ,sizeof(u_int32_t));
			index=-1;
		}
		else
		{
			index++;
		}

	}
	if (longitudLista==index)
	{
		sem_post(&sem_listMP);
		printf("No existe el segmento stack de este proceso \n");
		return 9999;
	}

	sem_post(&sem_listMP);
	return devuelta;
}
