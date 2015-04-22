#include "kernel.h"
#include <parser/parser.h>

#ifndef MANEJOSTACK_H_
#define MANEJOSTACK_H_

typedef struct {
	u_int32_t base;
	uint32_t offset;
	uint32_t longitud;
	uint32_t proceso;
} datos_a_enviar;

typedef struct {
	u_int32_t base;
	u_int32_t offset;
	u_int32_t valor;
	uint32_t proceso;
} datos_a_grabar;

u_int32_t ponerVariableEnStack(char identificador, u_int32_t digito);
u_int32_t buscarOffset(char identificador);
t_valor_variable buscarValor(u_int32_t digito, t_puntero direccion);
char* leerDatos (u_int32_t digito, u_int32_t base, u_int32_t offset, u_int32_t longitud,uint32_t proceso);
u_int32_t leerValor (u_int32_t digito, u_int32_t base, u_int32_t offset, u_int32_t longitud,uint32_t proceso);
char *serializarLeerDatos(u_int32_t digito, datos_a_enviar informacionParaEnviar);
void grabarValor(u_int32_t digito, u_int32_t base, u_int32_t offset, u_int32_t valor,uint32_t proceso);
char *serializarGrabarValor(u_int32_t digito, datos_a_grabar informacionParaEnviar);
void almacenarSinRetorno();
void leerFinalizar();
void almacenarConRetorno(t_puntero donde_retorno);
void leerRetorno(t_valor_variable valor_retorno);
t_dictionary* regenerarDiccionario(u_int32_t digito, PCB* pcb, t_dictionary *dic);

#endif /* MANEJOSTACK_H_ */
