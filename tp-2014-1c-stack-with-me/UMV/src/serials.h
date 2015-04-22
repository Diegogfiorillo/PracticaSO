/*
 * serials.h
 *
 *  Created on: 13/07/2014
 *      Author: utnso
 */

/*
 * serials.h
 *
 *  Created on: 13/07/2014
 *      Author: utnso
 */


#ifndef SERIALS_H_
#define SERIALS_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "funcionesUMV.h"
extern int claveContinuidadKernel;
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
	 u_int32_t pcodigo;
	 u_int32_t petiquetas;
	 u_int32_t pinstrucciones;
	 u_int32_t pstack;

} __attribute__((packed)) t_deUMVaKernel ;


//Estructura CPU y Kernel



int crearSocketServidor(char* puertoCliente);
t_deKernelAUMV* recibir_paquete_kernel(int s);
char* serializar_punteros_Kernel(char* serializado, u_int32_t a, u_int32_t b, u_int32_t c, u_int32_t d);
char* serializarVariable(char* serializado, char variable);
char* serializarContenido(char* serializado, int contenido);
char* serializarDireccionCPU(char* serializado, u_int32_t puntero);
t_Dir_deCPUaUMV* recibir_Direccion_CPU(int s);
t_Nombre_deCPUaUMV* recibir_Nombre_CPU(int s);
t_DirYValor_deCPUaUMV* recibir_DirYValor_CPU(int s);
t_variableYValor_deCPUaUMV* recibir_VariableYValor_CPU(int s);
u_int32_t recibirCodigoPeticion (int socket);

#endif /* SERIALS_H_ */

