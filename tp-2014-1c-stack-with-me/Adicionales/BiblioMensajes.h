/*
 * BiblioMensajes.h

 *
 *  Created on: 07/06/2014
 *      Author: StackWithMe
 */

#ifndef BIBLIOMENSAJES_H_
#define BIBLIOMENSAJES_H_


//Estructuras Kernel y UMV
typedef struct
{
	int id_proceso;
	int instrucciones_size;
	char* instrucciones;
	int codigo_size;
	char* codigo;
	int indice_etiquetas;
	char* etiquetas;

} t_deKernelAUMV ;

typedef struct
{
	char* pcodigo;
	char* petiquetas;
	char* pinstrucciones;
	char* pstack;
} t_deUMVaKernel ;

typedef struct
{
	int id_segmento;
} t_paraDestruir ;


//Estructuras UMV y CPU

typedef struct
{
	char* inicio;
	char* fin;
} t_peticion_indice_codigo ;

typedef struct
{
	char* linea;
} t_rta_indice_codigo ;

typedef struct
{
	char* posicion;
} t_peticion_stack;

typedef struct
{
	char* respuesta;
} t_rta_peticion_stack;

typedef struct
{
	char* mensaje;
}t_handshake;


typedef struct retornoUMV
{
	char* punteroCodigo;
	char* punteroEtiq;
	char* codIndex;			//este seria el indice de codigo
	char* stack;
}estructuraUMV;

//Estructura CPU y Kernel


void deserializarPCB(char **);
void deserializarUMV(char **);
int crearSocketCliente(char*, char*);
int crearSocketServidor(char*);
int sendall(int, char*, int*);
#endif /* BIBLIOMENSAJES_H_ */
