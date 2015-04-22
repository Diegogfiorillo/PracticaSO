/*
 * funcionesCPU.h
 *
 *  Created on: 17/07/2014
 *      Author: utnso
 */

#ifndef FUNCIONESCPU_H_
#define FUNCIONESCPU_H_
#include "commons/collections/list.h"


char* buscarLineaDeCodigoPedidaPorLaCPU(u_int32_t direccion, u_int32_t tamanio, u_int32_t offset, int socketCpu, u_int32_t idDeProceso);
void agregarVariablesAlStack(u_int32_t direccion, u_int32_t offset, char variable, u_int32_t disponibleStack,int socketDeCpu, u_int32_t idDeProceso);
void modificarVariablesEnElStack (u_int32_t direccion, u_int32_t offset, int modificacion,int socketDeCpu, u_int32_t idDeProceso);
int buscoElContenidoInt (u_int32_t direccion, u_int32_t offset, u_int32_t idDeProceso);
char buscoLaVariableChar (u_int32_t direccion, u_int32_t offset, u_int32_t idDeProceso);


#endif /* FUNCIONESCPU_H_ */
