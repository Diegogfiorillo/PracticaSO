#include <stdio.h>
#include <stdlib.h>
#include "kernel.h"
#include <parser/parser.h>

typedef struct estructura_puerto			//estructura que contiene los datos del archivo de puerto
{
	char* ipU;
	char* puertoU;
	char* ipK;
	char* puertoK;
	}Puerto;

typedef struct 			//estructura para contener los datos de cada linea procedente del pcb.codSegment
{
	u_int32_t offset;
	u_int32_t longitud;
}est_linea;

PCB* crearPcbVacio();
int recibirQuantum(int clienteSocket);
char* recibirPCB();
char* serializarIdentificadoryPosicion (u_int32_t digito, char identificador, u_int32_t base, t_puntero offset,u_int32_t proceso);
char* serializarPosicionyValor(u_int32_t base, t_valor_variable valor, t_puntero offset);
est_linea obtenerLinea();
char* serializarInfoLinea(u_int32_t base, u_int32_t desplazamiento, u_int32_t longitud, u_int32_t proceso);
void enviarInfoLinea(char* infoLinea);
Puerto* crear_estructura_config(char* path);
PCB* deserializarPCB(char* package);
int recibir_y_deserializar_PCB();
int crearSocketCliente(char* ipServidor, char* puertoServidor);
int sendall(int s, char* buffer, int* len);
