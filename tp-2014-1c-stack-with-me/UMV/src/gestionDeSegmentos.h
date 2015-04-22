/*
 * gestionDeSegmentos.h
 *
 *  Created on: 17/07/2014
 *      Author: utnso
 */

#ifndef GESTIONDESEGMENTOS_H_
#define GESTIONDESEGMENTOS_H_
#include "commons/collections/list.h"
#include <semaphore.h>
#include <sys/types.h>

typedef struct
{
	u_int32_t id_seg;
	u_int32_t dir_logica;
	u_int32_t longitud;
	char* memFisica;
	u_int32_t id_proceso;

}memP;

typedef struct
{
	char* fisica;
	u_int32_t longitud;
}vacios;

typedef struct
{
	u_int32_t logica;
	u_int32_t longitud;
}vaciosLogica;


memP* inicializarDatos(u_int32_t dirLogica, u_int32_t size, char* fisica, u_int32_t id_proc);
void grabaEnFisica(char* infoAGrabar, char* dondeGrabo, u_int32_t size);
void grabaValorEnFisica(int* infoAGrabar, char* dondeGrabo, u_int32_t size);
memP* crear_primer_nodo (u_int32_t size,u_int32_t id_proc,char* info);
bool segmentoAnterior(void * seg1, void * seg2);
t_list* listaDeVacios(t_list* list);
int compactar();
memP* agregar_segun_WF (u_int32_t id_proc, char* info, u_int32_t size);
memP* agregar_segun_FF(u_int32_t size, u_int32_t id_proc, char* info);
int destruirSegmento(u_int32_t id_seg_a_borrar, u_int32_t id_proc);
bool encontreSegmentos (void* id_proc1, void* id_proc2);
int crearSegmento(u_int32_t size, u_int32_t id_proc, char* info);
void destruirTodosLosSegmentos(u_int32_t id_proc);
u_int32_t generarLogicaRandom(u_int32_t id_proceso, u_int32_t tamanioDeSegmento);

#endif /* GESTIONDESEGMENTOS_H_ */
