#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <parser/parser.h>



#ifndef FUNCIONESPRIMITIVAS_H_
#define FUNCIONESPRIMITIVAS_H_


AnSISOP_funciones * ANSISOP_funciones_a_utilizar();
AnSISOP_kernel * ANSISOP_funciones_a_utilizar_kernel();

t_puntero definirVariable (t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable (t_nombre_variable identificador);
t_valor_variable dereferenciar (t_puntero direccionVariable);
void asignar (t_puntero direccionVariable, t_valor_variable valorVariable);
t_valor_variable obtenerValorCompartida (t_nombre_compartida variableCompartida);
t_valor_variable asignarValorCompartida (t_nombre_compartida variableCompartida, t_valor_variable valor);
void irAlLabel (t_nombre_etiqueta etiqueta);
void llamarSinRetorno (t_nombre_etiqueta etiqueta);
void llamarConRetorno (t_nombre_etiqueta etiqueta,t_puntero donde_retornar);
void finalizar (void);
void retornar (t_valor_variable retorno);
void imprimir(t_valor_variable valor_mostrar);
void imprimirTexto (char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void primitivaWait(t_nombre_semaforo identificador_semaforo);
void primitivaSignal(t_nombre_semaforo identificador_semaforo);
uint32_t obtenerLineaDeLabel( t_nombre_etiqueta t_nombre_etiqueta, char* etiquetas);
#endif /* FUNCIONESPRIMIIVAS_H_ */
