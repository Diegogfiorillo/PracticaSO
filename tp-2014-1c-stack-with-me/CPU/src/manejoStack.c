// FUNCIONES SOBRE EL STACK EN LAS PRIMITIVAS
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
#include "kernel.h"
#include "funcionesCpu.h"
#include "manejoStack.h"
#include "funcionesPrimitivas.h"

extern int clienteUMVCPU;
extern PCB* pcbAUtilizar;
extern int codigoFinalizacion;
extern int quantumRestante;
extern t_log* logger;
extern t_dictionary *diccionario;


/*
 /////////////////////////////////////////////////////////////////////////////////////////
 ////    																			  ////
 ////																				  ////
 //// si poniendo pcb.cursor_stack o algun otro puntero => poner &()pcb.cursor_stack  ////
 ////																				  ////
 ////																				  ////
 /////////////////////////////////////////////////////////////////////////////////////////
 *
 */

u_int32_t ponerVariableEnStack(char identificador, u_int32_t digito){

	u_int32_t puntero;
	//si ya esta en el diccionario => error
	if (buscarOffset(identificador)!=0){
		log_info(logger, "La variable ya esta registrada");
		return buscarOffset(identificador);
	}
	puntero = (pcbAUtilizar->cantVariables)*5; // posicion donde va a estar el valor
	char *infoSerializada = serializarIdentificadoryPosicion(digito, identificador, pcbAUtilizar->stackSegment, pcbAUtilizar->cursor_stack-pcbAUtilizar->stackSegment+puntero,pcbAUtilizar->processID);
	int tam = (4*sizeof(u_int32_t)+sizeof(char));
	int cantidadEnviada = send(clienteUMVCPU, infoSerializada, tam,0);
	if (cantidadEnviada==-1){
		log_error(logger, "Error la variable no se registro");
		quantumRestante=0;
		return 0;
	}
	u_int32_t validacion;
	recv(clienteUMVCPU, &validacion, sizeof(u_int32_t), 0);
	if(validacion==0){
		log_error(logger, "Segmentation fault al registrar la variable");
		quantumRestante=-1;
		codigoFinalizacion=12;
		return 0;
	}else if(validacion==1){
	//la registro en el diccionario
	puntero+=1;
	pcbAUtilizar->cantVariables+=1;

	char* nombreVar =malloc(2);
	char* referenciaNombre=nombreVar;
	nombreVar[0]=identificador;
	nombreVar[1]='\0';
	int *valor=malloc(sizeof(int));
	*valor=puntero;
	dictionary_put(diccionario,nombreVar, valor);
	free(referenciaNombre);
	}
	return puntero;

}

u_int32_t buscarOffset(char identificador){

	char* id =malloc(2);
	char* idPosta=id;
	id[0]=identificador;
	id[1]='\0';
	int* resultado;
	int puntero;
	if(dictionary_has_key(diccionario,id)){
		resultado=dictionary_get(diccionario, id);
		puntero= *resultado;
		return puntero;
	}
	free(idPosta);
	return 0;

}

t_valor_variable buscarValor(u_int32_t digito, t_puntero direccion){
	printf("Voy a buscar un valor\n");
	printf("En la direccion %d\n", direccion);
	printf("Tengo el cursor stack en %d\n", pcbAUtilizar->cursor_stack);
	printf("Tengo el stack segment en %d\n", pcbAUtilizar->stackSegment);
	printf("Tengo %d variables\n", pcbAUtilizar->cantVariables);
	u_int32_t tamanioContexto= (pcbAUtilizar->cantVariables)*5;
	if(direccion > tamanioContexto){
	char* val=leerDatos(digito,	pcbAUtilizar->stackSegment,direccion, sizeof(u_int32_t), pcbAUtilizar->processID);
	printf("Me llego: %s\n", val);
		if(val!=NULL){
		t_valor_variable valor;
		memcpy(&valor, val, sizeof(u_int32_t));
		printf("El valor de la variables es %d\n", valor);
		return valor;
		}else return -1;

	} else {
	char* val=leerDatos(digito,	pcbAUtilizar->stackSegment,pcbAUtilizar->cursor_stack-pcbAUtilizar->stackSegment+direccion, sizeof(u_int32_t),pcbAUtilizar->processID);
	t_valor_variable valor;
	memcpy(&valor, val, sizeof(u_int32_t));
	printf("El valor de la variables es %d\n", valor);
	return valor;
}
}

char* leerDatos (u_int32_t digito, u_int32_t base, u_int32_t offset, u_int32_t longitud, u_int32_t proceso){

	datos_a_enviar informacionParaEnviar;
	informacionParaEnviar.base = base;
	informacionParaEnviar.offset = offset;
	informacionParaEnviar.longitud = longitud;
	informacionParaEnviar.proceso=proceso;

	char* infoSerializada = serializarLeerDatos(digito, informacionParaEnviar);
	char* informacion=malloc(longitud);

	int tam = 5* sizeof(int);
	if(send(clienteUMVCPU, infoSerializada, tam,0)==-1){
		log_error(logger,"Error al leer datos");
		quantumRestante=0;
		return NULL;
	}

	u_int32_t verificacion;
	recv(clienteUMVCPU, &verificacion, sizeof(u_int32_t), 0);
	if(verificacion==0){
		log_error(logger, "Segmantation fault");
		quantumRestante=-1;
		codigoFinalizacion=12;
		return 0;
	}else if(verificacion==1){
	if(recv(clienteUMVCPU,informacion,longitud,0)==-1){
		log_error(logger,"Error al leer datos");
		quantumRestante=0;
		return NULL;
	}
	informacion[longitud]='\0';
	printf("me llego: %s\n", informacion);
	return informacion;
	}
}

u_int32_t leerValor(u_int32_t digito, u_int32_t base, u_int32_t offset, u_int32_t longitud,u_int32_t proceso){

char* respuesta =leerDatos(digito,base,offset,longitud,proceso);
int informacion;
memcpy(&informacion, respuesta, sizeof(u_int32_t));
printf("pedi un valor, es: %d", informacion);
return informacion;

}

char *serializarLeerDatos(u_int32_t digito, datos_a_enviar informacionParaEnviar){

	char * infoSerializada = malloc(5*sizeof(u_int32_t));
	u_int32_t offset = 0;
	u_int32_t size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(digito), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(informacionParaEnviar.base), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(informacionParaEnviar.offset), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(informacionParaEnviar.longitud), size_to_send);
	offset += size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(informacionParaEnviar.proceso), size_to_send);
	return infoSerializada;

}

void grabarValor(u_int32_t digito, u_int32_t base, u_int32_t offset, u_int32_t valor,u_int32_t proceso){


	datos_a_grabar informacionParaEnviar;
	informacionParaEnviar.base = base;
	informacionParaEnviar.offset = offset;
	informacionParaEnviar.valor = valor;
	informacionParaEnviar.proceso=proceso;
	char * infoSerializada = serializarGrabarValor(digito, informacionParaEnviar);
	int tam = 5*sizeof(u_int32_t);
	if(send(clienteUMVCPU, infoSerializada, tam,0)==-1){
		log_error(logger,"Error al almacenar datos");
		quantumRestante=0;
	}
	u_int32_t validacion;
	recv(clienteUMVCPU, &validacion, sizeof(u_int32_t), 0);
	if(validacion==0){
		log_error(logger,"Segmantation fault");
		quantumRestante=-1;
		codigoFinalizacion=12;
	}
}

char* serializarGrabarValor(u_int32_t digito, datos_a_grabar informacionParaEnviar){

	char * infoSerializada = malloc(5*sizeof(u_int32_t));
	u_int32_t offset = 0;
	u_int32_t size_to_send;
	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(digito), size_to_send);
	u_int32_t dig;
	memcpy(&dig, infoSerializada+offset, sizeof(u_int32_t));
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(informacionParaEnviar.base), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(informacionParaEnviar.offset), size_to_send);
	u_int32_t variable;
	memcpy(&variable, infoSerializada+offset, sizeof(u_int32_t));
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(informacionParaEnviar.valor), size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(infoSerializada + offset, &(informacionParaEnviar.proceso), size_to_send);
	return infoSerializada;

}

void almacenarSinRetorno(){//todo
printf("voy a almacenar sin retorno\n");
printf("la cantidad de variables es: %d\n", pcbAUtilizar->cantVariables);
u_int32_t offset = (pcbAUtilizar->cantVariables)*5+(pcbAUtilizar->cursor_stack)-(pcbAUtilizar->stackSegment);
	printf("guardo sin retorno el cursor stack:%d\n", pcbAUtilizar->cursor_stack);
	grabarValor(3,pcbAUtilizar->stackSegment, offset, (pcbAUtilizar->cursor_stack),pcbAUtilizar->processID);
	offset += 4;
	grabarValor(3,pcbAUtilizar->stackSegment, offset, pcbAUtilizar->progCounter,pcbAUtilizar->processID);
	offset += 4;
	pcbAUtilizar->cursor_stack += 2 *sizeof(u_int32_t)+(pcbAUtilizar->cantVariables)*5;
	pcbAUtilizar->cantVariables = 0;
	printf("Almaceno sin retorno\n");

}

void leerFinalizar(){
	printf("entro a leer finalizar\n");
	printf("el cursor stack es: %d",pcbAUtilizar->cursor_stack);
	printf("el stack segm es: %d",pcbAUtilizar->stackSegment);
	if (pcbAUtilizar->cursor_stack==pcbAUtilizar->stackSegment){
		int i;
		int offsetV=0;
		int offsetN=1;
		for(i=0; i<(pcbAUtilizar->cantVariables);i++){
			char* respuesta= malloc(sizeof(char));
			char* respuestala = malloc(11*sizeof(char));
			char* respuestaes = malloc(16*sizeof(char));
			respuestala="La Variable";
			respuestaes="tiene como valor";
			respuesta=leerDatos(1,pcbAUtilizar->stackSegment, offsetV, sizeof(char),pcbAUtilizar->processID);
			u_int32_t respuestaValor=leerValor(1, pcbAUtilizar->stackSegment,offsetN, sizeof(u_int32_t),pcbAUtilizar->processID);
			imprimirTexto(respuestala);
			imprimirTexto(respuesta);
			imprimirTexto(respuestaes);
			imprimir(respuestaValor);
			offsetV+=5;
			offsetN+=5;
		}
		quantumRestante = 0;
		codigoFinalizacion = 2;
		printf("Se ha finalizado el programa\n");
	}else{
		u_int32_t cursorViejo=pcbAUtilizar->cursor_stack-8;
		u_int32_t offset = (pcbAUtilizar->cursor_stack) - (pcbAUtilizar->stackSegment)- 4;
		pcbAUtilizar->progCounter = leerValor(1,pcbAUtilizar->stackSegment,offset, sizeof(u_int32_t),pcbAUtilizar->processID);
		offset -= 4;
		printf("Obtengo el program counter con valor %d\n", pcbAUtilizar->progCounter);
		pcbAUtilizar->cursor_stack = leerValor(1,pcbAUtilizar->stackSegment,offset, sizeof(u_int32_t),pcbAUtilizar->processID);
		printf("Obtengo el cursor del stack en %d\n", pcbAUtilizar->cursor_stack);
		u_int32_t cursor = pcbAUtilizar->cursor_stack;
		pcbAUtilizar->cantVariables = (cursorViejo-cursor) /5;
		printf("Por lo tanto la cantidad de variables que me quedan son:%d\n", pcbAUtilizar->cantVariables);
	}

}

void almacenarConRetorno(t_puntero donde_retorno){//todo
	printf("voy a almacenar con retorno\n");
	printf("la cantidad de variables es: %d\n", pcbAUtilizar->cantVariables);
	printf("El cursor del stack esta en:%d\n", pcbAUtilizar->cursor_stack);
	printf("El program counter esta en: %d\n", pcbAUtilizar->progCounter);
	printf("la funcion deberia retornar en: %d\n", donde_retorno);
	u_int32_t offset = (pcbAUtilizar->cantVariables)*5+(pcbAUtilizar->cursor_stack)-(pcbAUtilizar->stackSegment);
	grabarValor(3,pcbAUtilizar->stackSegment, offset, (pcbAUtilizar->cursor_stack),pcbAUtilizar->processID);
	offset += 4;
	grabarValor(3,pcbAUtilizar->stackSegment, offset, pcbAUtilizar->progCounter,pcbAUtilizar->processID);
	offset += 4;
	grabarValor(3,pcbAUtilizar->stackSegment, offset, donde_retorno,pcbAUtilizar->processID);
	offset += 4;
	pcbAUtilizar->cursor_stack += 3* sizeof(u_int32_t)+ (pcbAUtilizar->cantVariables)*5;
	pcbAUtilizar->cantVariables = 0;

}

void leerRetorno(t_valor_variable valor_retorno){

	u_int32_t base = (pcbAUtilizar->cursor_stack) - 4;
	u_int32_t offset= (pcbAUtilizar->cursor_stack)-(pcbAUtilizar->stackSegment)-4;
	printf("Saco el donde retorno, que es:%d\n", offset);
	u_int32_t offsetDondeGuardarValor = leerValor(1, pcbAUtilizar->stackSegment,offset,  sizeof(t_valor_variable),pcbAUtilizar->processID);
	printf("Deberia retornar en: %d\n", offsetDondeGuardarValor);
	base -= 4;
	offset-=4;
	pcbAUtilizar->progCounter = leerValor(1,pcbAUtilizar->stackSegment,offset, sizeof(u_int32_t),pcbAUtilizar->processID);
	printf("El program counter me queda en: %d\n", pcbAUtilizar->progCounter);
	base -= 4;
	offset-=4;
	pcbAUtilizar->cursor_stack = leerValor(1,pcbAUtilizar->stackSegment,offset, sizeof(u_int32_t),pcbAUtilizar->processID);
	u_int32_t cursor = pcbAUtilizar->cursor_stack;
	pcbAUtilizar->cantVariables = (base - cursor)/5;
	printf("Estoy guardando este valor: %d\n", valor_retorno);
	grabarValor(3, pcbAUtilizar->stackSegment, pcbAUtilizar->cursor_stack+offsetDondeGuardarValor-pcbAUtilizar->stackSegment, valor_retorno,pcbAUtilizar->processID);

}

t_dictionary* regenerarDiccionario(u_int32_t digito, PCB* pcb, t_dictionary *dic){

	u_int32_t desplazamiento=0;
	u_int32_t puntero;
	u_int32_t i;
	dictionary_clean(dic);

	for(i=0;i<pcb->cantVariables;i++){

		char* lectura = leerDatos(digito, pcb->stackSegment, pcb->cursor_stack-pcb->stackSegment+desplazamiento, 1, pcb->processID);
		char* id = malloc(2);
		id[0]=lectura[0];
		id[1]='\0';
		printf("Guardo la variable %s en el diccionario\n", id);
		u_int32_t *valor=malloc(sizeof(u_int32_t));
		*valor = desplazamiento +1;
		dictionary_put(dic, id, valor);
		desplazamiento+=5;
		free(id);

	}
	int cant=dictionary_size(dic);
	printf("Se regenero el dic con %d variables\n", cant);

	return dic;

}
