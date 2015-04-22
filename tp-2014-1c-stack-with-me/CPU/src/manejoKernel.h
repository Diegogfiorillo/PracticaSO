#include <parser/parser.h>
#include "kernel.h"

#ifndef MANEJOKERNEL_H_
#define MANEJOKERNEL_H_

char* serializarNombreVarCompartida(t_nombre_compartida nombre);
char* serializarNombreyValorVarCompartida(t_nombre_compartida nombre, t_valor_variable valor);
char* serializarValor(t_valor_variable valor, u_int32_t campoDelPCB);
char* serializarTexto(char* texto, u_int32_t campoDelPCB);
char* serializarTiempoyDispositivo(uint32_t tiempo, t_nombre_dispositivo dispositivo, PCB* pcbcito);
char* serializarSemaforoWait(t_nombre_semaforo identificador_semaforo, PCB* pcbcito);
char* serializarSemaforoSignal(t_nombre_semaforo identificador_semaforo);
char* serializarPCB(PCB*);

#endif /* MANEJOKERNEL_H_ */
