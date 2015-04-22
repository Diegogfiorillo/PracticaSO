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
#include <parser/metadata_program.h>
#include "kernel.h"
#include "manejoStack.h"
#include "manejoKernel.h"
#include "funcionesPrimitivas.h"
#include "funcionesCpu.h"



extern PCB* pcbAUtilizar;
extern int clienteKernelCPU;
extern int quantumRestante;
extern int codigoFinalizacion;
extern int clienteUMVCPU;
extern t_dictionary *diccionario;
extern t_log* logger;

AnSISOP_funciones * ANSISOP_funciones_a_utilizar(){

	AnSISOP_funciones * funciones = malloc(sizeof(AnSISOP_funciones));
	funciones->AnSISOP_definirVariable=&definirVariable;
	funciones->AnSISOP_obtenerPosicionVariable=&obtenerPosicionVariable;
	funciones->AnSISOP_dereferenciar=&dereferenciar;
	funciones->AnSISOP_asignar=&asignar;
	funciones->AnSISOP_obtenerValorCompartida=&obtenerValorCompartida;
	funciones->AnSISOP_asignarValorCompartida=&asignarValorCompartida;
	funciones->AnSISOP_irAlLabel=&irAlLabel;
	funciones->AnSISOP_llamarSinRetorno=&llamarSinRetorno;
	funciones->AnSISOP_llamarConRetorno=&llamarConRetorno;
	funciones->AnSISOP_finalizar=&finalizar;
	funciones->AnSISOP_retornar=&retornar;
	funciones->AnSISOP_imprimir=&imprimir;
	funciones->AnSISOP_imprimirTexto=&imprimirTexto;
	funciones->AnSISOP_entradaSalida=&entradaSalida;
	return funciones;

}

AnSISOP_kernel * ANSISOP_funciones_a_utilizar_kernel(){

	AnSISOP_kernel * funciones = malloc(sizeof(AnSISOP_kernel));
	funciones->AnSISOP_wait=&primitivaWait;
	funciones->AnSISOP_signal=&primitivaSignal;
	return funciones;

}



//1
t_puntero definirVariable (t_nombre_variable identificador_variable){
	if(quantumRestante!=-1){
	printf("Voy a definir Variable\n");
	printf("Defino la variable %c\n", identificador_variable);
	t_puntero puntero;
	puntero = ponerVariableEnStack(identificador_variable, 2);
	printf("La guardo en:%d\n", puntero);
	return puntero;
	}
}

//2
t_puntero obtenerPosicionVariable (t_nombre_variable identificador){
	if(quantumRestante!=-1){
	printf("Voy a obtener la posicion de una variable\n");
	printf("Estoy buscando la variable %c\n", identificador);
	t_puntero puntero;
	puntero = buscarOffset(identificador);
	printf("La encontre en: %d\n", puntero);
	if (puntero==0){
		log_info(logger,"La variable no esta registrada");
		return -1;
	}
	return puntero;
	}
}


//3
t_valor_variable dereferenciar (t_puntero direccionVariable){
	if(quantumRestante!=-1){
	printf("Voy a dereferenciar una variable\n");
	return buscarValor(1, direccionVariable);
	}
}

//4
void asignar (t_puntero direccionVariable, t_valor_variable valorVariable ){
	if(quantumRestante!=-1){
	printf("Voy a asignar un valor a una variable\n");
	printf("Voy a asignar en la direccion %d\n", direccionVariable);
	printf("Voy a asignar el valor: %d\n", valorVariable);
	u_int32_t tamContexto;
	tamContexto=(pcbAUtilizar->cantVariables)*5;
	if(direccionVariable>tamContexto)grabarValor(3, pcbAUtilizar->stackSegment,direccionVariable, valorVariable, pcbAUtilizar->processID);
	if(direccionVariable<tamContexto)grabarValor(3, pcbAUtilizar->stackSegment,pcbAUtilizar->cursor_stack-pcbAUtilizar->stackSegment+direccionVariable, valorVariable,pcbAUtilizar->processID);
	}
}


//5
t_valor_variable obtenerValorCompartida (t_nombre_compartida variableCompartida ){
	if(quantumRestante!=-1){
	printf("obtengo valor de una compartida\n");
	char *paqueteSerializado = serializarNombreVarCompartida(variableCompartida);
	int tamanio=(2*sizeof(u_int32_t)+strlen(variableCompartida));
	if(send(clienteKernelCPU, paqueteSerializado, tamanio,0)==-1){
		log_error(logger,"Error al enviar la variable compartida %s", variableCompartida);
		pcbAUtilizar->progCounter-=1;
		quantumRestante = 0;
		return 0;
	}
	printf("mande %d bytes\n", tamanio);
	t_valor_variable valor;
	if(recv(clienteKernelCPU, &valor, sizeof(u_int32_t), 0)==-1){
		log_error(logger,"Error al recibir el valor de la variable compartida %s", variableCompartida);
		pcbAUtilizar->progCounter-=1;
		quantumRestante = 0;
		return 0;
	}
	if(valor==-1){
		log_error(logger,"Error, no existe la variable compartida");
		quantumRestante=0;
	}
	printf("El valor es: %d", valor);
	return valor;
	//sacar el valor de la compartida
	}
}

//6
t_valor_variable asignarValorCompartida (t_nombre_compartida variableCompartida, t_valor_variable valor){
	if(quantumRestante!=-1){
	printf("asigno valor a una compartida\n");
	char *paqueteSerializado = serializarNombreyValorVarCompartida(variableCompartida, valor);
	int tamanio=(3*sizeof(u_int32_t)+strlen(variableCompartida));
	if(send(clienteKernelCPU, paqueteSerializado, tamanio,0)==-1){
		log_error(logger,"Error al enviar el valor de la variable compartida %s", variableCompartida);
		(pcbAUtilizar->progCounter)-=1;
		quantumRestante = 0;
		return 0;
	}
	int chequeo;
	recv(clienteKernelCPU, &chequeo, sizeof(u_int32_t), 0);
	if(chequeo==1){
		printf("La variable compartida se asigno correctamente");
	}
	return valor;
	}
}

//7
void irAlLabel (t_nombre_etiqueta etiqueta){
	if(quantumRestante!=-1){
	printf("Tengo que ir al label\n");
	int tamanio=strlen(etiqueta);
	if(etiqueta[tamanio]=='\0' &&etiqueta[tamanio-1]=='\n'){
		char* etiquetaPosta=malloc(tamanio-1);
		memcpy(etiquetaPosta, etiqueta, tamanio-1);
		etiquetaPosta[tamanio-1]='\0';
		etiqueta=etiquetaPosta;
	}
	printf("Para buscar la etiqueta: %s\n", etiqueta);
	char* informacionLineaSerializada = serializarInfoLinea(pcbAUtilizar->etiq_Segment , 0, pcbAUtilizar->tam_indice_etiquetas, pcbAUtilizar->processID);
	int tam = 5*sizeof(int);
	if(send(clienteUMVCPU,informacionLineaSerializada,tam,0)==-1){
		log_error(logger, "Error al enviar datos de la linea");
		pcbAUtilizar->progCounter-=1;
		quantumRestante = 0;
		return;
	}
	free(informacionLineaSerializada);

	u_int32_t verificacion;
	recv(clienteUMVCPU, &verificacion, sizeof(u_int32_t), 0);
	if(verificacion==0){
			quantumRestante=0;
			codigoFinalizacion=12;
	}else if(verificacion==1){
	char* etiquetas = malloc(pcbAUtilizar->tam_indice_etiquetas);

	if(recv(clienteUMVCPU, etiquetas, pcbAUtilizar->tam_indice_etiquetas , 0)==-1){
		log_error(logger, "Error al recibir datos de la linea");
		pcbAUtilizar->progCounter-=1;
		quantumRestante = 0;
		return;
	}else{
		printf("Las etiquetas: %s\n", etiquetas);
		t_puntero_instruccion puntero = metadata_buscar_etiqueta(etiqueta, etiquetas, pcbAUtilizar->tam_indice_etiquetas);
		pcbAUtilizar->progCounter=(puntero);
		printf("La siguiente linea a buscar despues del label es: %d\n", puntero);
	}

	}
	}
}

//8
void llamarSinRetorno (t_nombre_etiqueta etiqueta){
	if(quantumRestante!=-1){
	printf("Me llaman sin retorno\n");
	printf("cant dic :%d\n", dictionary_size(diccionario));
	almacenarSinRetorno();
	irAlLabel(etiqueta);		//cambiar el programcounter
	diccionario = regenerarDiccionario(1, pcbAUtilizar, diccionario);
	}
}

//9

void llamarConRetorno (t_nombre_etiqueta etiqueta,t_puntero donde_retornar){
	if(quantumRestante!=-1){
	printf("Me llaman con retorno\n");
	almacenarConRetorno(donde_retornar);
	irAlLabel(etiqueta);
	diccionario = regenerarDiccionario(1, pcbAUtilizar, diccionario);
	}
}

//10

void finalizar (void){
	if(quantumRestante!=-1){
	printf("Debo finalizar\n");
	leerFinalizar();
	diccionario = regenerarDiccionario(1, pcbAUtilizar, diccionario);		//regenero el diccionario con las variables y sus offset
	}
}

//11

void retornar (t_valor_variable retorno){
	if(quantumRestante!=-1){
	printf("Debo retornar\n");
	leerRetorno(retorno);
	diccionario = regenerarDiccionario(1, pcbAUtilizar, diccionario);
	}
}

//12

void imprimir(t_valor_variable valor_mostrar){
	if(quantumRestante!=-1){
	printf("Debo imprimir una variable\n");
	char *paqueteSerializado = serializarValor(valor_mostrar, pcbAUtilizar->socketPrograma);
	int tamanio=3*(sizeof(u_int32_t));
	if(send(clienteKernelCPU, paqueteSerializado, tamanio,0)==-1){
		log_error(logger,"Error al enviar valor para imprimir");
		pcbAUtilizar->progCounter-=1;
		quantumRestante=0;
	}
	}
}

//13

void imprimirTexto (char* texto){
	if(quantumRestante!=-1){
	printf("Debo imprimir texto\n");
	char* paqueteSerializado = serializarTexto(texto, pcbAUtilizar->socketPrograma);
	int tamanio=(3*sizeof(u_int32_t)+strlen(texto));
	if(send(clienteKernelCPU, paqueteSerializado, tamanio, 0)==-1){
		log_error(logger,"Error al enviar texto para imprimir");
		pcbAUtilizar->progCounter-=1;
		quantumRestante=0;
	}
	}
}

//14

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){
	if(quantumRestante!=-1){
	printf("Debo hacer una operacion de entrada/salida\n");
	pcbAUtilizar->prioridad = 2;
	char* paqueteSerializado = serializarTiempoyDispositivo(tiempo, dispositivo, pcbAUtilizar);
	int tamanio=3*sizeof(u_int32_t)+strlen(dispositivo)+sizeof(PCB);
	if(send(clienteKernelCPU, paqueteSerializado, tamanio, 0)==-1){
		log_error(logger,"Error al enviar dispositivo %s para i/o", dispositivo);
		pcbAUtilizar->progCounter-=1;
	}
	quantumRestante = 0;
	codigoFinalizacion = 3;
	}
}

//15

void primitivaWait(t_nombre_semaforo identificador_semaforo){
	if(quantumRestante!=-1){
	printf("Debo hacer wait\n");
	pcbAUtilizar->prioridad = 2;
//	identificador_semaforo[strlen(identificador_semaforo)]='\0';
	printf("El semaforo a serializar es: %s y su tamanio es %d\n", identificador_semaforo, strlen(identificador_semaforo));
	char * paqueteSerializado = serializarSemaforoWait(identificador_semaforo, pcbAUtilizar);
	int tamanio=(2*sizeof(u_int32_t)+strlen(identificador_semaforo)+sizeof(PCB));
	printf("Voy a enviar %d bytes\n", tamanio);
	if(send(clienteKernelCPU, paqueteSerializado, tamanio,0)==-1){
		quantumRestante = 0;
		codigoFinalizacion = 8;
		log_error(logger,"Error al enviar wait del semaforo %s\n", identificador_semaforo);
		pcbAUtilizar->progCounter-=1;
		return;
	}

	int valor;
	if(recv(clienteKernelCPU, &valor, sizeof(u_int32_t) , 0)==-1){
		quantumRestante = 0;
		codigoFinalizacion = 8;
		log_error(logger,"Error al recibir wait del semaforo %s", identificador_semaforo);
		pcbAUtilizar->progCounter-=1;
		return;
	}
	printf("Me devuelve el valor %d del semaforo\n", valor);
	if(valor==0){
		quantumRestante = 0;
		codigoFinalizacion = 8;
		log_info(logger,"No se permite ejecutar wait del semaforo %s", identificador_semaforo);
	}
	if(valor==-1){
		quantumRestante=0;
		log_error(logger, "No existe el semaforo");
	}
	}
}

//16

void primitivaSignal(t_nombre_semaforo identificador_semaforo){
	if(quantumRestante!=-1){
	printf("Debo hacer signal\n");
	char * paqueteSerializado = serializarSemaforoSignal(identificador_semaforo);
	int tamanio=(2*sizeof(u_int32_t)+strlen(identificador_semaforo));
	if(send(clienteKernelCPU, paqueteSerializado, tamanio,0)==-1){
		log_error(logger,"Error al enviar signal del semaforo %s", identificador_semaforo);
		pcbAUtilizar->progCounter-=1;
		quantumRestante = 0;
	}
	}
}

