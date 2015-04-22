/*
 * FuncionesPCP.c

 *
 *  Created on: 28/04/2014
 *      Author: federico
 */


#include <stdio.h>
#include <stdlib.h>
#include "FuncionesPCP.h"
#include "FuncionesPLP.h"
#include <string.h>
#include <errno.h>
#include <commons/collections/list.h>

pthread_mutex_t Compartidas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Semaforos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ColaSemaforosBloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Dispositivos= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCpu = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_cpu;
extern t_queue* colaCpuDisponibles;
fd_set socket_Cpus, socketDeCpus;

t_list* crear_cola_exec()
{
	t_list* colaProcesoEjec = list_create();
	return colaProcesoEjec;
}

t_queue* crear_cola_block()
{
	t_queue* colaProcesoBloqueado = queue_create();
	return colaProcesoBloqueado;
}

char* serializarPCB(PCB* pcb)
{
		char* pcbserializado = malloc(12*sizeof(u_int32_t));

		u_int32_t offset = 0;
		u_int32_t size_to_send;										//El orden de serializado es el siguiente:


		size_to_send =  sizeof(u_int32_t);						//Segundo, el id de proceso
		memcpy(pcbserializado + offset, &(pcb->processID), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);					//Tercero, el program counter
		memcpy(pcbserializado + offset, &(pcb->progCounter), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);					//Cuarto, el segmento de stack
		memcpy(pcbserializado + offset, &(pcb->stackSegment), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);						//Quinto, el segmento de codigo
		memcpy(pcbserializado + offset, &(pcb->codSegment), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);					//Sexto, el segmento de etiquetas
		memcpy(pcbserializado + offset,&(pcb->etiq_Segment), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);					//Septimo, el cursor del stack
		memcpy(pcbserializado + offset, &(pcb->cursor_stack), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);					//Octavo, la cantidad de variables
		memcpy(pcbserializado + offset, &(pcb->cantVariables), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);					//Noveno, el contexto Actual
		memcpy(pcbserializado + offset, &(pcb->codIndex), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);				//Decimo, el tamañno del indice de etiquetas
		memcpy(pcbserializado + offset, &(pcb->tam_indice_etiquetas), size_to_send);
		offset += size_to_send;

		size_to_send =  sizeof(u_int32_t);							//Onceavo, y ultimo, el peso
		memcpy(pcbserializado + offset, &(pcb->peso), size_to_send);
		offset += size_to_send;

		size_to_send= sizeof(u_int32_t);
		memcpy(pcbserializado + offset, &(pcb->socketPrograma), size_to_send);
		offset += size_to_send;

		size_to_send= sizeof(u_int32_t);
		memcpy(pcbserializado + offset, &(pcb->prioridad), size_to_send);
		return pcbserializado;
}


int encolar_proceso_listo()
{
	pthread_mutex_lock(&MutexExec);
	PCB* pcb= (PCB*)list_remove(colaExec,0);
	pthread_mutex_unlock(&MutexExec);
	pthread_mutex_lock(&MutexListos);
	int pos = list_add(colaListos,pcb);
	pthread_mutex_unlock(&MutexListos);
	return pos;
}

int obtengoDigitoOperacionCpu(char* copia){
	int digito;
	memcpy(&digito, copia, sizeof(u_int32_t));					//saco el digito
	return digito;
}


int atiendoCpus()
{
		int i, socketCliente;
		bool finalizo = false;
		struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
		socklen_t addrlen = sizeof(addr);

		int maxfd = socketCpu;
		FD_ZERO(&socketDeCpus);
		FD_ZERO(&socket_Cpus);
		FD_SET(socketCpu, &socketDeCpus);

		do {
			memcpy(&socket_Cpus, &socketDeCpus, sizeof(fd_set));

			int variable = select(maxfd + 1, &socket_Cpus, NULL, NULL, NULL);
			if (variable) {
				//si es nueva conexion, agrego a la lista
				for (i = 0; i < maxfd + 1; i++) {

					if (FD_ISSET(i, &socket_Cpus)) {

						if (i == socketCpu) {
							do {
								socketCliente = accept(socketCpu,
										(struct sockaddr *) &addr, &addrlen);
								if (socketCliente < 0) {
									if (errno != EWOULDBLOCK) {
										log_error(logger,
												"Error al aceptar conexion entrante");
										finalizo = true;
															   }
									break;
														}
								log_info(logger, "Se acepto una nueva CPU");
								if (maxfd < socketCliente)
									maxfd = socketCliente;
								FD_SET(socketCliente, &socketDeCpus);
								} while (socketCliente!=-1);
						} else {

							int digito;
							int datosRecibidos = recv(i, &digito,
									sizeof(u_int32_t), 0);
									if(digito==5){
										FD_CLR(i,&socket_Cpus);
										FD_CLR(i,&socketDeCpus);
									}
							if (datosRecibidos > 0) atender_cpu(i,digito);   //todo hacer hilo
						}			//else
					} //ISSET
				}			//for

			}			//if
		} while (finalizo == false);

		for (i = 0; i <= maxfd; ++i) {
				if (FD_ISSET(i, &socketDeCpus)){
					close(i);
				FD_CLR(i, &socketDeCpus);
			}
		}
		return -1;
	}

void atender_cpu(int s, int digitoDeOperacion){
	printf("Me llego la operacion %d\n", digitoDeOperacion);
	printf("Del socket: %d\n", s);
	if(digitoDeOperacion>0) {
		switch(digitoDeOperacion){
		case 1:					//Cpu libre
		{
			printf("empiezo el case 1\n");
			PCB* pcbVacio=deserializarPCB(s);
			if(!list_is_empty(colaListos))
			{
				printf("Hay una CPU libre\n");
				int envioQuantum=send(s, &quantum, sizeof(u_int32_t),0);
				if(envioQuantum==-1) log_info(logger, "El quantum no se ha enviado correctamente");
				printf("Envie quantum\n");
				pthread_mutex_lock(&MutexListos);
				PCB* unPcb = list_get(colaListos,0);
				char* serializado = serializarPCB(unPcb);
				send(s,serializado,sizeof(PCB),0);
			}
			else
			{
				t_CPU* socki= malloc(sizeof(t_CPU));
				socki->socket=s;
				pthread_mutex_lock(&mutexCpu);
				queue_push(colaCpuDisponibles,socki);
				pthread_mutex_unlock(&mutexCpu);
			}
			list_iterate(colaCpuDisponibles->elements,mostrar_contenido);
			printf("termino el case 1\n");
		}
			break;
		case 2:				//Finalizo Programa
		{
			printf("empiezo el case 2\n");
			PCB* pcb2 = deserializarPCB(s);		//deserializo
			bool find_by_id(void* parametro)
			{
				PCB* pcb = (PCB*) parametro;
				return pcb->processID==pcb2->processID;
			}
			if(list_any_satisfy(colaExec, find_by_id)){
			buscar_y_SacarDeExec(pcb2->processID);
			mostrar_cola_exec();
			enviarEnd(pcb2->socketPrograma);
			meterEnExit(pcb2);
			mostrar_cola_exit();
			}
			printf("termino el case 2\n");
		}
			break;
		case 3:				//Se bloqueo por i/o
		{
			printf("empiezo el case 3\n");
			t_Disp* dispositivo= deserializoDispositivo(s);
			bool find_by_id(void* parametro)
				{
				PCB* pcb = (PCB*) parametro;
				return (pcb->processID)==(dispositivo->pcb->processID);
				}
			if(list_any_satisfy(colaExec, find_by_id)){
			buscar_y_SacarDeExec( dispositivo->pcb->processID);
			encolar_proceso_block( dispositivo->pcb);
			mostrar_cola_exec();
			mostrar_cola_block();
			encolar_en_dispositivo(dispositivo->dispositivo, dispositivo->tiempo, dispositivo->pcb->processID); //TODO
			}
			printf("termino el case 3\n");
		}
			break;
		case 4:		//interrumpidoPorQuantum
		{
			printf("empiezo el case 4\n");
			mostrar_cola_exec();
			PCB* pcb4 = deserializarPCB(s);		//deserializo
			bool find_by_id(void* parametro)
				{
					PCB* pcb = (PCB*) parametro;
					return (pcb->processID)==(pcb4->processID);
				}
			if(list_any_satisfy(colaExec, find_by_id)){
			buscar_y_SacarDeExec(pcb4->processID);
			pthread_mutex_lock(&MutexListos);
			list_add(colaListos, pcb4);
			pthread_mutex_unlock(&MutexListos);
			mostrar_cola_exec();
			mostrar_cola_listos();
			sem_post(&sem_pcp);
			}
			printf("termino el case 4\n");
		}
			break;
		case 5:			//CPU Caido
		{
			printf("empiezo el case 5\n");
			PCB* pcb5 = deserializarPCB(s);		//deserializo
			bool find_by_id(void* parametro)
				{
					PCB* pcb = (PCB*) parametro;
					return (pcb->processID)==(pcb5->processID);
				}
			if(list_any_satisfy(colaExec, find_by_id)){
			buscar_y_SacarDeExec(pcb5->processID);
			aviso_programa(pcb5->socketPrograma);
			meterEnExit(pcb5);
			close(s);
			}
			printf("termino el case 5\n");
		}
			break;
		case 6:				//asignar variable compartida
		{
			printf("empiezo el case 6\n");
			t_compartida* varCom= deserializoVariableCompartida(s);
			asignoVarCompartida(varCom->var, varCom->valor);
			int dig=1;
			send(s, &dig, sizeof(u_int32_t), 0);
			printf("termino el case 6\n");
		}
			break;
		case 7:				//dar valor compartida
		{
			printf("empiezo el case 7\n");
			t_pedidoCompartida* pedCom= deserializoPedidoCompartida(s);
			int valor = obtengoValor(pedCom->var);
			printf("El valor que obtuve del diccionario de compartidas es: %d", valor);
			send(s, &valor, sizeof(u_int32_t), 0);
			printf("termino el case 7\n");
		}
			break;
		case 8:				//wait
		{
			printf("empiezo el case 8\n");
			t_Semaforo* est_sem = deserializoSemaforo(s);
			wait_sem(s,est_sem);
			printf("termino el case 8\n");
		}
			break;
		case 9:					//signal
		{
			printf("empiezo el case 9\n");
			t_SemaforoSig* est_sem2 = deserializoSemaforoSignal(s);
			signal_sem(s,est_sem2);
			printf("termino el case 9\n");
		}
			break;
		case 10:			//imprimir texto
		{
			printf("empiezo el case 10\n");
			t_texto* txt_imprimir= deserializoTexto(s);
			if((enviar_digito_imprimir(txt_imprimir->socketPrograma,1))==-1)
			{
				log_error(logger,"Error al enviar digito de impresion al programa.");
			}
			printf("Me llega un texto %s  con tamanio %d\n", txt_imprimir->texto, txt_imprimir->tamanio);
			char* paquete= serializoTexto(txt_imprimir);
			int size=(sizeof(u_int32_t)+txt_imprimir->tamanio);
			int envioCorrecto = send(txt_imprimir->socketPrograma, paquete, size,0);
			if(envioCorrecto<=0)
			{
			 	 perror("Error al enviar texto al programa");
			}
			printf("termino el case 10\n");
		}
			break;
		case 11:			//imprimir variable
		{
			printf("empiezo el case 11\n");
			t_Variable* var_imprimir= deserializoVariable(s);
			if((enviar_digito_imprimir(var_imprimir->socketPrograma,2))==-1)
			{
				log_error(logger,"Error al enviar digito de impresion al programa.");
			}
			int size2=(sizeof(u_int32_t));
			int envioCorrecto2 = send(var_imprimir->socketPrograma, &(var_imprimir->valor), size2,0);
			if(envioCorrecto2<=0)
			{
				perror("Error al enviar variable al programa");
			}
			printf("termino el case 11\n");
		}
			break;
		case 12:				//ocurre seg fault
		{
			printf("empiezo el case 12\n");
				PCB* pcbSeg= deserializarPCB(s);
				bool find_by_id(void* parametro)
					{
						PCB* pcb = (PCB*) parametro;
						return (pcb->processID)==(pcbSeg->processID);
					}
				if(list_any_satisfy(colaExec, find_by_id)){
				u_int32_t digito=5;
				int envioCorrecto3 = send(pcbSeg->socketPrograma, &digito, sizeof(u_int32_t),0);
				if(envioCorrecto3<=0)
				 	{
				 	 perror("Error al enviar segmentation fault");
					 }
				buscar_y_SacarDeExec( pcbSeg->processID);
				meterEnExit(pcbSeg);
				}
				printf("termino el case 12\n");
		}
				break;
		default:
			log_error(logger, "Mala conexion con el Cpu");
			break;
		}
		printf("Termino una operacion\n");
	}
	usleep(retardo);
}

void enviarEnd(int socketPrograma){
	u_int32_t digito=3;
	int envioCorrecto = send(socketPrograma, &digito, sizeof(u_int32_t),0);
	if(envioCorrecto<=0)
	 	{
			 	 perror("Error al enviar la finalizacion del programa");
	 	}
}
int obtengoValor(char* variable) //para mi aca esta el problema
{

	int valor;
	int* aux;
	int tamanio= strlen(variable);
	printf("Me llega la variable: %s , con el tamanio: %d\n", variable, tamanio);
	if(variable[tamanio]=='\0')printf("Llego con barra cero en el ultimo\n");
	if(variable[tamanio-1]=='\0')printf("Llego con barra cero en el anteultimo\n");
	if(variable[tamanio-2]=='\0')printf("Llego con barra cero en el anteanteultimo\n");
	if(variable[tamanio-1]=='\r')printf("Llego con barra ere en el anteultimo\n");
	if(variable[tamanio-2]=='\r')printf("Llego con barra ere en el anteanetultimo\n");
	if(variable[tamanio-1]=='\n')printf("Llego con barra ene en el anteultimo\n");
	if(variable[tamanio-2]=='\n')printf("Llego con barra ene en el anteanteultimo\n");
	if(variable[tamanio+1]=='\r')printf("Llego con barra ere en el postultimo\n");
	if(variable[tamanio+1]=='\n')printf("Llego con barra ene en el postultimo\n");
	if(variable[tamanio]=='\0' && variable[tamanio-1]=='\n'){
			char* variablePosta=malloc(tamanio-1);
			memcpy(variablePosta, variable, tamanio-1);
			variablePosta[tamanio-1]='\0';
			variable= variablePosta;
		}
	char* var= malloc(tamanio);
	memcpy(var, variable, tamanio);

	if (dictionary_has_key(dicVarCompartidas, var )){
	aux = dictionary_get(dicVarCompartidas, var);
	valor = *aux;
	return valor;
	}else if (dictionary_has_key(dicVarCompartidas, variable)){
	aux = dictionary_get(dicVarCompartidas, variable);
	valor = *aux;
	return valor;
	}else return -1;
}
t_Disp* deserializoDispositivo(int s)
{
	t_Disp* dispositivo = malloc(sizeof(t_Disp));
	u_int32_t size_entero = sizeof(u_int32_t);
	u_int32_t status;

	status = recv(s,&(dispositivo->tamanio),size_entero,0);
	if(!status)return 0;

	printf("El tamanio del dispositivo es: %d\n", dispositivo->tamanio);

	dispositivo->dispositivo=malloc(dispositivo->tamanio+1);
	status = recv(s,dispositivo->dispositivo,(dispositivo->tamanio),0);
	if(!status)return 0;
	dispositivo->dispositivo[dispositivo->tamanio]='\0';
	printf("El dispositivo es: %s\n", dispositivo->dispositivo);

	status = recv(s,&(dispositivo->tiempo),size_entero,0);
	if(!status)return 0;
	printf("El tiempo que me pide es: %d\n", dispositivo->tiempo);

	dispositivo->pcb=deserializarPCB(s);

	return dispositivo;
}
char* serializoSegFault()
{
	char* segFault="SegmentationFault";
	u_int32_t tam= strlen(segFault);
	char* serializado = malloc(sizeof(u_int32_t)+tam);
	u_int32_t size_to_send;
	u_int32_t offset;

	size_to_send =  sizeof(u_int32_t);
	memcpy(serializado, &tam, size_to_send);
	offset += size_to_send;

	size_to_send = tam;
	memcpy(serializado+offset,segFault,tam);

	return serializado;
}
char* serializoTexto(t_texto* texto){
	char* serializado= malloc(sizeof(u_int32_t)+texto->tamanio);
	u_int32_t offset = 0;
	u_int32_t size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(serializado, &(texto->tamanio), size_to_send);
	offset += size_to_send;

	size_to_send =  texto->tamanio;
	memcpy(serializado + offset, texto->texto, size_to_send);
	return serializado;
}

char* serializoVariable(t_Variable* variable){
	char* serializado= malloc(2*sizeof(u_int32_t)+variable->tamanio);
	u_int32_t offset = 0;
	u_int32_t size_to_send;


	size_to_send =  sizeof(u_int32_t);
	memcpy(serializado, &(variable->tamanio), size_to_send);
	offset += size_to_send;

	size_to_send =  variable->tamanio;
	memcpy(serializado + offset, variable->var, size_to_send);
	offset += size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(serializado, &(variable->valor), size_to_send);

	return serializado;
}

t_texto* deserializoTexto(int s)
{
	t_texto* texto= malloc(sizeof(t_texto));
	u_int32_t size_entero = sizeof(u_int32_t);
	int status;

	status = recv(s,&(texto->tamanio),size_entero,0);
	if(!status)return 0;
	texto->texto=malloc(texto->tamanio);
	status = recv(s,texto->texto,texto->tamanio,0);
	if(!status)return 0;

	status = recv(s,&(texto->socketPrograma),size_entero,0);
	if(!status)return 0;

	return texto;
}

t_Variable* deserializoVariable(int s)
{
	t_Variable* variable= malloc(sizeof(t_Variable));
	u_int32_t size_entero = sizeof(u_int32_t);
	u_int32_t status;

	status = recv(s,&(variable->valor),size_entero,0);
	if(!status)return 0;

	status = recv(s,&(variable->socketPrograma),size_entero,0);
	if(!status)return 0;
	printf("recibo el valor: %d", variable->valor);
	printf("recibo el socketprog: %d", variable->socketPrograma);


	return variable;
}

t_pedidoCompartida* deserializoPedidoCompartida(int s)
{
	t_pedidoCompartida* variable = malloc(sizeof(t_pedidoCompartida));

	u_int32_t offset = 0;
	u_int32_t size_entero = sizeof(u_int32_t);
	u_int32_t status;

	status = recv(s,&(variable->tamanio),size_entero,0);
	if(!status)return 0;

	variable->var= malloc(variable->tamanio);
	status = recv(s,variable->var,(variable->tamanio),0);
	variable->var[variable->tamanio]='\0';
	if(!status)return 0;


	return variable;
}

t_compartida* deserializoVariableCompartida(int s)
{
	t_compartida* variable = malloc(sizeof(t_compartida));
	u_int32_t offset = 0;
	u_int32_t size_entero = sizeof(u_int32_t);
	u_int32_t status;

	status = recv(s,&(variable->tamanio),size_entero,0);
	if(!status)return 0;

	variable->var= malloc(variable->tamanio);
	status = recv(s,variable->var,(variable->tamanio),0);
	variable->var[variable->tamanio]='\0';


	status = recv(s,&(variable->valor),size_entero,0);
	if(!status)return 0;

	return variable;
}
t_Semaforo* deserializoSemaforo(int s)
{
	t_Semaforo* semaforo = malloc(sizeof(t_Semaforo));
	u_int32_t size_entero = sizeof(u_int32_t);
	u_int32_t status;

	status = recv(s,&(semaforo->tamanio),size_entero,0);
	if(!status)return 0;
	semaforo->sem=malloc((semaforo->tamanio)+1);
	status = recv(s,semaforo->sem,semaforo->tamanio,0);
	if(!status)return 0;

	semaforo->sem[semaforo->tamanio]='\0';
	if(semaforo->sem[semaforo->tamanio]=='\0' && semaforo->sem[semaforo->tamanio-1]=='\n'){
				char* variablePosta=malloc((semaforo->tamanio)-1);
				memcpy(variablePosta, semaforo->sem, (semaforo->tamanio)-1);
				variablePosta[(semaforo->tamanio)-1]='\0';
				semaforo->sem= variablePosta;
				printf("hice el adaptador\n");
			}
	printf("el nombre del semaforo tiene tamanio %d\n", semaforo->tamanio);
	printf("Recibi el semaforo %s\n", semaforo->sem);
	semaforo->pcb=deserializarPCB(s);

	return semaforo;
}

t_SemaforoSig* deserializoSemaforoSignal(int s)
{
	t_SemaforoSig* semaforo = malloc(sizeof(t_SemaforoSig));
	u_int32_t size_entero = sizeof(u_int32_t);
	u_int32_t status;

	status = recv(s,&(semaforo->tamanio),size_entero,0);
	if(!status)return 0;
	semaforo->sem=malloc((semaforo->tamanio)+1);
	status = recv(s,semaforo->sem,semaforo->tamanio,0);
	if(!status)return 0;

	semaforo->sem[semaforo->tamanio]='\0';
	if(semaforo->sem[semaforo->tamanio]=='\0' && semaforo->sem[semaforo->tamanio-1]=='\n'){
			char* variablePosta=malloc((semaforo->tamanio)-1);
			memcpy(variablePosta, semaforo->sem, (semaforo->tamanio)-1);
			variablePosta[(semaforo->tamanio)-1]='\0';
			semaforo->sem= variablePosta;
			printf("hice el adaptador\n");
	}

	return semaforo;
}
void asignoVarCompartida(char* variable, int valor)
{
	int* aux;
	aux = dictionary_get(dicVarCompartidas, variable);
	*aux = valor;
	}
void buscar_y_SacarDeExec(int ide){
	bool find_by_id(void* parametro)
	{
		PCB* pcb = (PCB*) parametro;
		return pcb->processID==ide;
	}

	if(!list_is_empty(colaExec) && list_any_satisfy(colaExec,find_by_id))
	{
		pthread_mutex_lock(&MutexExec);
		PCB* pcb2 = list_remove_by_condition(colaExec,find_by_id);
		free(pcb2);
		pthread_mutex_unlock(&MutexExec);
	}
}

void encolar_en_dispositivo(char* dispositivo, int tiempo, int identif)
{
	printf("Aca me llega el dispositivo %s\n", dispositivo);
	if(dictionary_has_key(dicDispositivos, dispositivo)){

	IO* nodo = dictionary_get(dicDispositivos, dispositivo);
	t_disp* proc = malloc(2* sizeof(u_int32_t));
	proc->idP=identif;
	proc->tiempo=tiempo;

	pthread_mutex_lock(&nodo->mutex);
	queue_push(nodo->bloqueados, proc);
	pthread_mutex_unlock(&nodo->mutex);

	sem_post(&(nodo->semaforo));
	}else printf("No se encontro el dispositivo\n");
}

//PCB* sacarDeListos(){
//
//	bool find_by_id(void* parametro)
//	{
//		PCB* pcb = (PCB*) parametro;
//		return pcb->processID==ide;
//	}
//	if(list_any_satisfy(colaListos,find_by_id))
//	pthread_mutex_lock(&MutexListos);
////	list_sort(colaListos, OrdenoPorPrioridad);	//saca el primero de listos, y lo pone en ejecucion
//	PCB* pcb = (PCB*) list_remove(colaListos, 0);
//	pthread_mutex_unlock(&MutexListos);
//	return pcb;
//}

void meterEnExec( PCB* pcb){
	pthread_mutex_lock(&MutexExec);
	list_add(colaExec, pcb);
	pthread_mutex_unlock(&MutexExec);
}
int envioCpu (PCB* pcb, int s){
	char* paquete= serializarPCB(pcb);
	int envioCorrecto = send(s, paquete,sizeof(PCB),0);                           //mandar el PCB al cpu
	if(envioCorrecto<=0)
	{
		perror("Error al enviar pcb al CPU");
	}
	return envioCorrecto;
}

int destruir_segmentos(PCB* unPcb){
	u_int32_t programa=unPcb->processID;
	char* paquete= serializarDestruir(programa);
	int size=4 * sizeof(u_int32_t);
	int envioCorrecto = send(socketUmv, paquete, size,0);
	return envioCorrecto;
}

char* serializarDestruir(u_int32_t ide) {

	char *paqueteSerializado = malloc(4 * sizeof(u_int32_t));

	int offset = 0;
	int size_to_send;
	int tam = 0;
	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, &(ide), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, &(tam), size_to_send);
	offset += size_to_send;


	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, &(tam),	size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(u_int32_t);
	memcpy(paqueteSerializado + offset, &(tam),	size_to_send);
	offset += size_to_send;

	return paqueteSerializado;
}

PCB* deserializarPCB(int s)
{
	PCB* pcb = malloc(sizeof(PCB));

	int size_entero = sizeof(u_int32_t);
	int status;


	status = recv(s,&(pcb->processID),size_entero,0);
	if(!status)return 0;

	status = recv(s,&(pcb->progCounter),size_entero,0);
	if(!status)return 0;

	status = recv(s,&(pcb->stackSegment),size_entero,0);
	if(!status)return 0;

	status = recv(s,&(pcb->codSegment),size_entero,0);
	if(!status)return 0;


	status = recv(s,&(pcb->etiq_Segment),size_entero,0);
	if(!status)return 0;


	status = recv(s,&(pcb->cursor_stack),size_entero,0);
	if(!status)return 0;


	status = recv(s,&(pcb->cantVariables),size_entero,0);
	if(!status)return 0;


	status = recv(s,&(pcb->codIndex),size_entero,0);
	if(!status)return 0;


	status = recv(s,&(pcb->tam_indice_etiquetas),size_entero,0);
	if(!status)return 0;

	status = recv(s,&(pcb->peso),size_entero,0);
	if(!status)return 0;


	status = recv(s,&(pcb->socketPrograma),size_entero,0);
	if(!status)return 0;


	status = recv(s,&(pcb->prioridad),size_entero,0);
	if(!status)return 0;

	printf("El PCB me llego con estos datitos:\n");
	printf("id:%d\n", pcb->processID);
	printf("PC:%d\n", pcb->progCounter);
	printf("seg cod:%d\n", pcb->codSegment);
	printf("seg etiq:%d\n", pcb->etiq_Segment);
	printf("cod index:%d\n", pcb->codIndex);
	printf("cantidad variables:%d\n", pcb->cantVariables);
	printf("cursor stack:%d\n", pcb->cursor_stack);
	printf("seg stack:%d\n", pcb->stackSegment);
	printf("cant etiquetas:%d\n", pcb->tam_indice_etiquetas);
	printf("peso:%d\n", pcb->peso);
	printf("prioridad:%d\n", pcb->prioridad);
	printf("socketPrograma:%d\n", pcb->socketPrograma);


	return pcb;

}

void encolar_proceso_block( PCB* pcb)
{
	pthread_mutex_lock(&MutexBlock);
	queue_push(colaBlock, pcb);
	pthread_mutex_unlock(&MutexBlock);
}

PCB* sacar_proceso_block( int ide)
{
	bool find_by_id(void* parametro)
		{
		PCB* pcb = (PCB*) parametro;
		return (pcb->processID)==ide;
		}
	pthread_mutex_lock(&MutexBlock);
	PCB* pcbObtenido = list_remove_by_condition(colaBlock->elements,find_by_id);
	pthread_mutex_unlock(&MutexBlock);
	return pcbObtenido;
}
void signal_sem(int s, t_SemaforoSig* semaforo)
{
	if(dictionary_has_key(dicSemaforos, semaforo->sem)){
	t_semaforos* semaforoObtenido = dictionary_get(dicSemaforos,semaforo->sem);

	semaforoObtenido->valor++;
	printf("El valor despues del signal del semaforo %s, es: %d\n", semaforo->sem, semaforoObtenido->valor);
	if((semaforoObtenido->valor)<=0)
	{
		int finalizo=1;

		while(((!queue_is_empty(semaforoObtenido->bloqSem)) && finalizo!=0)){

		pthread_mutex_lock(&ColaSemaforosBloqueados);
		int* PunteroID = queue_pop(semaforoObtenido->bloqSem);
		int identificador = *PunteroID;
		printf("Necesito sacar el id: %d\n", identificador);
		pthread_mutex_unlock(&ColaSemaforosBloqueados);
		bool find_by_id(void* parametro)
			{
			PCB* pcb = (PCB*) parametro;
			return (pcb->processID)==identificador;
			}
		if (list_any_satisfy(colaBlock->elements, find_by_id)){
		printf("Saco un pcb de block\n");
		PCB* pcb = sacar_proceso_block(identificador);
		mostrar_cola_block();
		pthread_mutex_lock(&MutexListos);
		list_add(colaListos,pcb);
		pthread_mutex_unlock(&MutexListos);
		sem_post(&sem_pcp);
		mostrar_cola_listos();
		finalizo=0;
		} else semaforoObtenido->valor++;

		}//while
	}//if valor menor a 0
	}//if esta en el diccionario
}
void wait_sem(int s, t_Semaforo* semaforo)
{
	int digito;
	if(dictionary_has_key(dicSemaforos, semaforo->sem)){
		printf("Coincide el nombre del semaforo\n");
	t_semaforos* semaforoObtenido = dictionary_get(dicSemaforos, semaforo->sem);
	semaforoObtenido->valor--;
	printf("El valor del wait de semaforo  %s queda en: %d\n", semaforo->sem, semaforoObtenido->valor);
	if((semaforoObtenido->valor)<0)
	{
		bool find_by_id(void* parametro)
			{
				PCB* pcb = (PCB*) parametro;
				return (pcb->processID)==(semaforo->pcb->processID);
			}
		if(list_any_satisfy(colaExec, find_by_id)){
		buscar_y_SacarDeExec(semaforo->pcb->processID);
		encolar_proceso_block(semaforo->pcb);
		digito=0;
		send(s,&digito,sizeof(u_int32_t),0);
		pthread_mutex_lock(&ColaSemaforosBloqueados);
		queue_push(semaforoObtenido->bloqSem, &(semaforo->pcb->processID));
		pthread_mutex_unlock(&ColaSemaforosBloqueados);
		}else {
			semaforoObtenido->valor++;
			digito=0;
			send(s,&digito,sizeof(u_int32_t),0);
		}
	}
	else
	{
		digito=1;
		send(s,&digito,sizeof(u_int32_t),0);
	}
	}
}

int enviar_digito_imprimir(int socketPrograma,int digito)
{
	return send(socketPrograma,&digito,sizeof(u_int32_t),0);
}


void mostrar_contenido(void* socket)//funciona de closure en el list_iterate
{
	printf("Socket: %d\n", ((t_CPU*) socket)->socket);
	printf(" \n");
}

void puedoPasarACpu()
{
	printf("El tamaño de la cola es: %d \n", queue_size(colaCpuDisponibles));
	if(!queue_is_empty(colaCpuDisponibles))
	{
		printf("Tengo una cpu esperando al pedo\n");
		pthread_mutex_lock(&mutexCpu);
		t_CPU* socki = queue_pop(colaCpuDisponibles);
		pthread_mutex_unlock(&mutexCpu);
		pthread_mutex_lock(&MutexListos);
		PCB* pcb = list_get(colaListos,0);
		pthread_mutex_unlock(&MutexListos);
		printf("Hay una CPU libre\n");
		int envioQuantum=send(socki->socket, &quantum, sizeof(u_int32_t),0);
		if(envioQuantum==-1) log_info(logger, "El quantum no se ha enviado correctamente");
		printf("Envie quantum\n");
		char* serializado = serializarPCB(pcb);
		send(socki->socket,serializado,sizeof(PCB),0);
	}
}
