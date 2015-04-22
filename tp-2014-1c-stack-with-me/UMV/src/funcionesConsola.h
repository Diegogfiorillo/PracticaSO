/*
 * funcionesConsola.h
 *
 *  Created on: 08/06/2014
 *      Author: utnso
 */

#ifndef FUNCIONESCONSOLA_H_
#define FUNCIONESCONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/collections/list.h"
#include "commons/log.h"
#include <sys/types.h>
#include <semaphore.h>
#include "commons/config.h"
//
typedef struct
{
	t_log* logger;
	t_log* logConsola;
}hiloConsola;

typedef struct
{
	t_log* logger;
	char* puerto;
}hiloKernelyCPU;

typedef struct
{
	t_log* logger;
	int socketsIndividuales;
}hiloCadaCPU;
//
////funciones CONSOLA

void mostrarContenidoPorArchivo(int base,int offset, int tamanio, char* baseTotal, FILE* archivo);
void mostrarBytePorBytePorArchivo(int cantidad, char * mem, FILE* archivo);
void mostrarBytePorByte(int cantidad, char * mem);
void mostrarContenido(int base,int offset, int tamanio, char* baseTotal);
void funcionOperacionSM();
void funcionOperacionEB();
void funcionOperacionC(t_log* logger);
void funcionOperacionD(t_log* logger);
void funcionOperacion (t_log* logger);
void funcionRetardo (t_log* logger);
void funcionCambiarAlgoritmo(t_log* logger);
void funcionCompactar(t_log* logger);
void funcionDumpSEConArch (FILE* archivoDump);
void funcionDumpSESinArch ();
void funcionDumpCONT(FILE* archivoDump, char respuesta[]);
void imprimirSegmentosPorProceso(int id_proc, char deseaArchivo[], FILE* archivoDump);
bool ordenarPorProceso (void* seg1, void* seg2);
void mostrarTablasDeTodos(FILE* archivoDump, char deseaArchivo[]);
void funcionDumpTS(FILE* archivoDump, char deseaArchivo[]);
void funcionDump(FILE* archivoDump);



#endif /* FUNCIONESCONSOLA_H_ */
