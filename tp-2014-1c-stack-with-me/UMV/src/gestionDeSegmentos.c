/*
 * gestionDeSegmentos.c
 *
 *  Created on: 17/07/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "commons/collections/list.h"
#include <string.h>
#include "gestionDeSegmentos.h"
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

extern char* codAlg;

extern char* base;
extern char* memLogica;

extern t_list* listMP;
u_int32_t id_seg = 0;
extern u_int32_t size_mp;
extern u_int32_t disponible;

extern sem_t sem_disponible;
extern pthread_mutex_t sem_memLogica;
extern sem_t sem_listMP;
extern sem_t sem_idseg;


memP* inicializarDatos(u_int32_t dirLogica, u_int32_t size, char* fisica, u_int32_t id_proc)
{

	memP* datos=malloc(sizeof(memP));
	datos->id_seg = id_seg;
	datos->dir_logica = dirLogica;
	datos->longitud= size;
	datos->memFisica = fisica;
	datos->id_proceso = id_proc;

	return datos;
}

void grabaEnFisica(char* infoAGrabar, char* dondeGrabo, u_int32_t size)
{
	 memcpy(dondeGrabo,infoAGrabar,size);

}

void grabaValorEnFisica(int* infoAGrabar, char* dondeGrabo, u_int32_t size)
{
	 memcpy(dondeGrabo,infoAGrabar,size);
}

memP* crear_primer_nodo (u_int32_t size,u_int32_t id_proc,char* info)
{
	sem_wait(&sem_listMP);
	u_int32_t dirLogicaRandom=rand();
	memP* datos = inicializarDatos(dirLogicaRandom,size, base, id_proc);
	list_add(listMP, datos);
	grabaEnFisica(info,base,size);

	pthread_mutex_lock(&sem_memLogica);
	memLogica+=size;
	pthread_mutex_unlock(&sem_memLogica);


	disponible-=size;
	sem_post(&sem_listMP);

	return datos;
}

bool segmentoAnterior(void * seg1, void * seg2)
{
	memP* segmento1=(memP*) seg1;
	memP* segmento2=(memP*) seg2;

	return segmento1->memFisica < segmento2->memFisica;
}

t_list* listaDeVacios(t_list* list)
{
	t_list * listaVacia = list_create();
	if (list_size(list) == 0)                            //si no hay nodos en la lista principal
	{
		vacios* segmento=malloc(sizeof(vacios));
		segmento->fisica=base;
		segmento->longitud=size_mp;

		list_add(listaVacia,segmento);

	}
	else
	{
		memP* primerSegmento = (memP*) list_get(list, 0);
		if(primerSegmento->memFisica!=base)                    //si el primer segmento no coincide con la base
		{
			vacios* segmento=malloc(sizeof(vacios));
			segmento->fisica=base;
			segmento->longitud=(primerSegmento->memFisica - base);
			list_add(listaVacia,segmento);
		}

	u_int32_t i=0;
	for (i = 0; i < list_size(list)-1; i++)
	{
		memP* segmento1 = (memP*) list_get(list, i);
		memP* segmento2 = (memP*) list_get(list,i+1);

		if ((segmento1->memFisica+segmento1->longitud) != (segmento2->memFisica) )   // si no coinciden principio y fin
		{
			vacios* segmentoIntermedio = malloc(sizeof(vacios));
			segmentoIntermedio->fisica=(segmento1->memFisica+segmento1->longitud);
			segmentoIntermedio->longitud=(segmento2->memFisica)-(segmento1->memFisica+segmento1->longitud);
			list_add(listaVacia, segmentoIntermedio);
		}
	}

	memP* ultimoSegmento = (memP*) list_get(list,list_size(list)-1);
	if ( ultimoSegmento->memFisica+ultimoSegmento->longitud != (base+size_mp) )
	{
		vacios* segmentoFinal=malloc(sizeof(vacios));
		segmentoFinal->fisica=(ultimoSegmento->memFisica+ultimoSegmento->longitud);
		segmentoFinal->longitud=(base+size_mp)-segmentoFinal->fisica;
		list_add(listaVacia, segmentoFinal);
	}


	}//del else

	return listaVacia;
}

char* funcionCompactarAchetada()
{
	sem_wait(&sem_listMP);
	if(list_size(listMP)==0)
	{
		char* vacia=malloc(sizeof(char*));
		vacia="empt";
		sem_post(&sem_listMP);
		return vacia;
	}
	list_sort(listMP,segmentoAnterior);
	char* ultimoLugar=malloc(sizeof(char*));
	ultimoLugar="null";
	memP* segmento=list_get(listMP,0);
	if(segmento->memFisica!=base)
	{
		memP* actualizado=segmento;
		actualizado->memFisica=base;
		list_replace(listMP,0,actualizado);
		memcpy(actualizado->memFisica,segmento->memFisica,segmento->longitud);
		ultimoLugar=base+actualizado->longitud;
	}
	int i=1;
	for(i=1;i<list_size(listMP); i++)
	{
		memP* segmento=list_get(listMP,i);
		memP* actualizado=segmento;
		actualizado->memFisica=ultimoLugar;
		list_replace(listMP,i,actualizado);
		memcpy(ultimoLugar,actualizado->memFisica,actualizado->longitud);
		ultimoLugar+=actualizado->longitud;
	}
	sem_post(&sem_listMP);
	return ultimoLugar;
}

//int compactar()
//{
//	sem_wait(&sem_listMP);
//	sem_wait(&sem_disponible);
//	if(disponible==0)
//	{
//		sem_post(&sem_disponible);
//		sem_post(&sem_listMP);
//		return -1;
//	}
//	else
//	{
//
//		int longLista=list_size(listMP);
//		u_int32_t dirCompactada;
//
//		if(longLista==0)
//		{
//			sem_post(&sem_disponible);
//			sem_post(&sem_listMP);
//			return -2;
//		}
//		else
//		{
//			if(longLista==1)
//			{
//				memP* unicoSeg=list_get(listMP,0);
//				if(unicoSeg->memFisica==base)
//				{
//					printf("La memoria no necesita compactarse \n");
//					dirCompactada=unicoSeg->dir_logica+unicoSeg->longitud;
//					sem_post(&sem_disponible);
//					sem_post(&sem_listMP);
//					return dirCompactada;
//				}
//				else
//				{
//					memcpy(base,unicoSeg->memFisica,unicoSeg->longitud);
//					memP* datosActualizados=unicoSeg;
//					datosActualizados->memFisica=base;
//					datosActualizados->dir_logica = 0;
//					list_replace(listMP,0,datosActualizados);
//					dirCompactada=unicoSeg->dir_logica+unicoSeg->longitud;
//					sem_post(&sem_listMP);
//					sem_post(&sem_disponible);
//					return dirCompactada;
//				}
//			}
//			else
//			{
//			list_sort(listMP,&segmentoAnterior);
//			int j=0;
//			for(j=0;j<longLista;j++)
//			{
//				memP* segActual = malloc(sizeof(memP));
//				memP* auxActual= segActual;
//
//				memP* segSiguiente = malloc(sizeof(memP));
//				memP* auxSiguiente= segSiguiente;
//
//				segActual=list_get(listMP,j);
//
//				if(segActual->memFisica!=base && j==0)    //por si esta desfazado
//				{
//					memP* datosActualizados0= segActual;
//					memcpy(base,segActual->memFisica,segActual->longitud);
//					datosActualizados0->memFisica=base;
//					datosActualizados0->dir_logica = 0;
//					list_replace(listMP,j,datosActualizados0);
//				}
//
//				if(j+1<longLista)
//				{
//					segSiguiente = list_get(listMP,j+1);
//					if(segActual->memFisica+segActual->longitud!=segSiguiente->memFisica)
//					{
//						memP* datosActualizados1 = segSiguiente;
//						memcpy(segActual->memFisica+segActual->longitud,segSiguiente->memFisica,segSiguiente->longitud);
//						datosActualizados1->memFisica=segActual->memFisica+segActual->longitud;
//						datosActualizados1->dir_logica = (segActual->memFisica+segActual->longitud)-base;
//						list_replace(listMP,j+1,datosActualizados1);
//						dirCompactada=datosActualizados1->dir_logica+datosActualizados1->longitud;
//					}
//				}
//				else
//				{
//					memP* segActual=list_get(listMP,longLista-1);
//					memP* segSiguiente=list_get(listMP,longLista-2); //no es el siguiente, sino el anterior
//
//					memP* datosActualizadosUltimo=segActual;
//					memcpy(segActual->memFisica,segActual->memFisica,segActual->longitud);
//					datosActualizadosUltimo->memFisica=segSiguiente->memFisica+segSiguiente->longitud;
//					datosActualizadosUltimo->dir_logica=segSiguiente->dir_logica+segSiguiente->longitud;
//					list_replace(listMP,longLista-1,datosActualizadosUltimo);
//
//					dirCompactada=datosActualizadosUltimo->dir_logica+datosActualizadosUltimo->longitud;
//				}
//
//				free(auxActual);
//				free(auxSiguiente);
//
//			} //del for
//			}
//		} //del else
//		sem_post(&sem_disponible);
//		sem_post(&sem_listMP);
//		printf("\nCompactando la Memoria Principal\n");
//		printf("\n");
//		usleep(1500000);
//		printf("▓▓▓▓▓░░▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓\n"
//				"▓▓▓▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓▓▓░▓▓░░▓▓▓▓▓░▓▓▓▓▓▓▓▓▓░░▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓▓▓░▓▓▓▓\n"
//				"▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓░░▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓\n"
//				"▓▓▓▓▓░▓▓▓▓▓▓▓░▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n");
//		printf("\n");
//		usleep(1500000);
//		printf("▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
//				"▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
//				"▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
//				"▓▓▓▓▓░▓▓▓▓▓▓▓░▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓░▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n");
//		printf("\n");
//		usleep(1500000);
//		printf("▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
//				"▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
//				"▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
//				"▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n");
//
//		printf("\n");
//		return dirCompactada;
//	}
//}


t_list* listaDeLogicasVacios(t_list* list)
{
	t_list * listaVacia = list_create();

	memP* primerSegmento = (memP*) list_get(list, 0);
		if(primerSegmento->dir_logica!=0)                    //si el primer segmento no coincide con la base
		{
			vaciosLogica* segmento=malloc(sizeof(vaciosLogica));
			segmento->logica=0;
			segmento->longitud=(primerSegmento->dir_logica - 0);
			list_add(listaVacia,segmento);
		}

	u_int32_t i=0;
	for (i = 0; i < list_size(list)-1; i++)
	{
		memP* segmento1 = (memP*) list_get(list, i);
		memP* segmento2 = (memP*) list_get(list,i+1);

		if ((segmento1->dir_logica+segmento1->longitud) != (segmento2->dir_logica) )   // si no coinciden principio y fin
		{
			vaciosLogica* segmentoIntermedio = malloc(sizeof(vaciosLogica));
			segmentoIntermedio->logica=(segmento1->dir_logica+segmento1->longitud);
			segmentoIntermedio->longitud=(segmento2->dir_logica)-(segmento1->dir_logica+segmento1->longitud);
			list_add(listaVacia, segmentoIntermedio);
		}
	}

	memP* ultimoSegmento = (memP*) list_get(list,list_size(list)-1);
	if ( ultimoSegmento->dir_logica+ultimoSegmento->longitud != RAND_MAX )
	{
		vaciosLogica* segmentoFinal=malloc(sizeof(vaciosLogica));
		segmentoFinal->logica=(ultimoSegmento->dir_logica+ultimoSegmento->longitud);
		segmentoFinal->longitud=RAND_MAX-segmentoFinal->logica;
		list_add(listaVacia, segmentoFinal);
	}

	return listaVacia;
}


u_int32_t generarLogicaRandom(u_int32_t id_proceso, u_int32_t tamanioDeSegmento)
{
	u_int32_t direccion = rand();

//	u_int32_t gen=(RAND_MAX*size_mp)/direccion;

	printf("direccion random %d \n", direccion);
	//sem_wait(&sem_listMP);
	u_int32_t direccionADevolver;


	bool filtrarPorProceso (void* seg)
	{
		memP* segmento = (memP*) seg;
		return segmento->id_proceso == id_proceso;
	}

	bool segmentoAnteriorLogica(void * seg1, void * seg2)
	{
		memP* segmento1=(memP*) seg1;
		memP* segmento2=(memP*) seg2;

		return segmento1->dir_logica < segmento2->dir_logica;
	}

	t_list* listaDePrograma=list_filter(listMP,filtrarPorProceso);
	list_sort(listaDePrograma,segmentoAnteriorLogica);

	int w=0;
	for(w=0; w<list_size(listaDePrograma); w++)
	{
		memP* segmento=list_get(listaDePrograma,w);
		if (direccion==segmento->dir_logica)      //si coinciden no me sirve
		{
			direccion=rand();
			w=0;   //para que vuelva a iterar
		}
		else                                    //si no coinciden, valido
		{
			t_list* listaVaciosLogica =listaDeLogicasVacios(listaDePrograma);
			int i =0;
			u_int32_t loQueOcupa=direccion+tamanioDeSegmento;
			for(i=0; i<list_size(listaVaciosLogica); i++)
			{
				vaciosLogica* espacioLogico=list_get(listaVaciosLogica,i);
				printf("dir logica de vacios %d \n", espacioLogico->logica);
				printf("longitud de vacios %d \n", espacioLogico->longitud);
				if(loQueOcupa>=espacioLogico->logica   &&    loQueOcupa< (espacioLogico->logica+espacioLogico->longitud) )
				{
					printf("Encuentra la direccion\n");
					direccionADevolver=direccion;
					i=list_size(listaVaciosLogica);
					w=list_size(listaDePrograma);
				}
			//else, sigue con el for
			}//dle for

		}//del else
	}
//	sem_post(&sem_listMP);
	return direccionADevolver;
}



memP* agregar_segun_FF(u_int32_t size, u_int32_t id_proc, char* info)
{
	sem_wait(&sem_listMP);
	sem_wait(&sem_disponible);
	t_list* espaciosVacios=listaDeVacios(listMP);
		vacios* espacio=malloc(sizeof(vacios));

	u_int32_t i =0;

	for (i = 0; i < list_size(espaciosVacios); i++)
	{
		espacio=(vacios*)list_get(espaciosVacios,i);

		if (espacio->longitud >= size)
		{
			char* fisicaAGrabar=espacio->fisica;
			u_int32_t dirLogica = generarLogicaRandom(id_proc,size);

			memP* nuevoSegmento = inicializarDatos(dirLogica,size,fisicaAGrabar,id_proc);
			list_add(listMP,nuevoSegmento);

			grabaEnFisica(info,nuevoSegmento->memFisica,nuevoSegmento->longitud);
			list_destroy_and_destroy_elements(espaciosVacios,free);


			disponible-=size;
			sem_post(&sem_disponible);

			sem_post(&sem_listMP);
			return nuevoSegmento;
		}
		else
		{
			//nothing
		}
	}
	//si no entro en ninguno de los huecos...

	char* primeraDirDisponibleFisica = funcionCompactarAchetada();
	u_int32_t dirLogica = generarLogicaRandom(id_proc,size);
	memP* nuevoSegmento = inicializarDatos(dirLogica,size,primeraDirDisponibleFisica,id_proc);

	list_add(listMP,nuevoSegmento);

	grabaEnFisica(info,nuevoSegmento->memFisica,nuevoSegmento->longitud);
	list_destroy_and_destroy_elements(espaciosVacios,free);

	disponible-=size;
	sem_post(&sem_disponible);

	sem_post(&sem_listMP);
	return nuevoSegmento;
}

memP* agregar_segun_WF (u_int32_t id_proc, char* info, u_int32_t size)
{
	sem_wait(&sem_listMP);
	sem_wait(&sem_disponible);
	list_sort(listMP,&segmentoAnterior);

	t_list* espaciosVacios=listaDeVacios(listMP);
	vacios* espacio;

	u_int32_t max=0;
	char* fisicaDelMax;
	u_int32_t i=0;
	for(i=0; i<list_size(espaciosVacios); i++)
	{
		espacio=(vacios*)list_get(espaciosVacios,i);
		if(espacio->longitud>max)
		{
			max=espacio->longitud;
			fisicaDelMax=espacio->fisica;
		}
	}

	memP* nuevoSegmento ;
	if(max<size)
	{
		char* primeraDirDisponibleFisica = funcionCompactarAchetada();
		u_int32_t dirLogica = generarLogicaRandom(id_proc,size);
		nuevoSegmento = inicializarDatos(dirLogica,size,primeraDirDisponibleFisica,id_proc);
	}
	else
	{
		char* fisicaAGrabar=fisicaDelMax;
		u_int32_t dirLogica = generarLogicaRandom(id_proc,size);
		nuevoSegmento = inicializarDatos(dirLogica,size,fisicaAGrabar,id_proc);
	}

	list_add(listMP,nuevoSegmento);

	grabaEnFisica(info,nuevoSegmento->memFisica,size);
	list_destroy_and_destroy_elements(espaciosVacios,free);

	disponible-=size;
	sem_post(&sem_disponible);

	sem_post(&sem_listMP);
	return nuevoSegmento;
}


int destruirSegmento(u_int32_t id_seg_a_borrar, u_int32_t id_proc)
{
	printf("El id a destruir es: %d\n", id_proc);
	sem_wait(&sem_listMP);
	int longList=list_size(listMP);

	int clave =0;
	int i=0;
	int index=0;

	while(i<longList)
	{
		memP* datos=malloc(sizeof(memP));
		memP* aux=datos;
		datos=list_get(listMP,i);
		if (datos->id_proceso==id_proc && datos->id_seg==id_seg_a_borrar)
		{
			sem_wait(&sem_disponible);
			disponible+=datos->longitud;
			sem_post(&sem_disponible);
			list_remove_and_destroy_element(listMP,index,free);

			clave = 1;
			i=longList+1;
		}
		else
		{
			i++;
			index++;
		}
		free(aux);
	}

	sem_post(&sem_listMP);
	return clave;
}

void destruirTodosLosSegmentos(u_int32_t id_proc)
{
	bool segmentosCoinciden(void* segmento)
	{
		memP* seg=(memP*)segmento;
		return seg->id_proceso==id_proc;
	}
	int j=0;
	sem_wait(&sem_listMP);
	int longitudLista=list_size(listMP);
	for(j=0; j<longitudLista; j++)
	{
		list_remove_and_destroy_by_condition(listMP,segmentosCoinciden,free);
	}
	sem_post(&sem_listMP);
}



int crearSegmento(u_int32_t size, u_int32_t id_proc, char* info)
{
	sem_wait(&sem_idseg);
	if (size>0)
	{
	printf(" %d\n", size);
	printf(" %d\n", disponible);
	if (size <= disponible)
	{
		int index=0;
		int lista_size=list_size(listMP);
		memP* segmentoCreado;

		while(index<lista_size ) //si la lista NO esta vacia
		{

			if (strcmp(codAlg,"FF")==0) //AGREGO CON FIRST-FIT
			{
				segmentoCreado = agregar_segun_FF(size,id_proc, info);
			}
			else //AGREGO CON WORST-FIT
			{
				segmentoCreado = agregar_segun_WF(id_proc,info,size);
			}
			//actualizo tamaño total de memoria principal

			id_seg++;
			sem_post(&sem_idseg);

			return segmentoCreado->dir_logica;

		} //del while

		segmentoCreado = crear_primer_nodo(size,id_proc, info);

		id_seg++;
		sem_post(&sem_idseg);

		return segmentoCreado->dir_logica;

	}//del segundo if
	else
	{
		printf("Memory Overload \nEl segmento a guardar no entra en memoria.\n");  //no entra en memoria
		sem_post(&sem_idseg);
		return -1;
	}
	}else if(size==0){
		sem_post(&sem_idseg);
		return 0;
	}else
	{
		printf("No se puede guardar un segmento de tamanio negativo.\n");
		sem_post(&sem_idseg);
		return -1;
	}

	return EXIT_SUCCESS;
}
