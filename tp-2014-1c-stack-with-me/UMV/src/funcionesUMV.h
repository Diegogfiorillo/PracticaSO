/*
 * funcionesUMV.h
 *
 *  Created on: 10/07/2014
 *      Author: utnso
 */

#ifndef FUNCIONESUMV_H_
#define FUNCIONESUMV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/collections/list.h"
#include "funcionesConsola.h"
#include "commons/log.h"
#include <sys/types.h>
#include <semaphore.h>
#include "commons/config.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
extern sem_t sem_disponible;
extern pthread_mutex_t sem_memLogica;
extern sem_t sem_listMP;
extern sem_t sem_idseg;
extern u_int32_t tamanio_stack;
extern u_int32_t disponible;

extern t_list* listMP;


typedef struct
{
	char nombre_variable;
	u_int32_t direccion;
	u_int32_t offset;
	u_int32_t idDeProceso;
} t_Nombre_deCPUaUMV ;
//no se responde; la cpu nos manda la variable que quiere agregar la stack y la direccion logica de
//de donde se encuentra el  stack y el offset de la posicion dentro del stack de la variable

typedef struct
{
	u_int32_t direccionDelaProximaLineaAEjecutar;
	u_int32_t offset;
	u_int32_t tamanio;
	u_int32_t idDeProceso;
} t_Dir_deCPUaUMV;
//devuelvo la linea a ejecutar, la direccion que nos manda es la logica,
typedef struct
{
	char* linea;
	u_int32_t idDeProceso;
} t_Dir_UMVaCPU;
//DEVUELVO LA LINEA DE CODIGO

typedef struct
{
	u_int32_t direccion_variable;
	int modificaion;
	u_int32_t offset;
	u_int32_t idDeProceso;
} t_DirYValor_deCPUaUMV;
//devuelvo nada; nos manda la direccion fisica de la variable a modificar y respectiva modificacion
typedef struct
{
	u_int32_t direccion_variable;
	u_int32_t offset;
	u_int32_t idDeProceso;
} t_variableYValor_deCPUaUMV;
//me manda la direccion de la variable y el offset, y le devuelvo el nombre de la variable y su contenido
typedef struct
{
	char variable;
	u_int32_t idDeProceso;
} t_variable_deUMVaCPU;

//Le respondo con el nombre de la variable
typedef struct
{
	int contenido;
	u_int32_t idDeProceso;

} t_valor_deUMVaCPU;

//Le respondo con el  contenido

void  *Kernel_function(void*);
void  *Console_function(void*);
void *hiloCpu(void*);
void *CPU_function(void*);

#endif /* FUNCIONESUMV_H_ */
