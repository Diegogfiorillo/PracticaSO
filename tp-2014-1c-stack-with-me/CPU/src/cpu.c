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
#include "ejecucion.h"
#include "funcionesCpu.h"
#include "funcionesPrimitivas.h"
#include "kernel.h"
#include "manejoStack.h"
#include "manejoKernel.h"


#define MAXDATASIZE 100

int clienteKernelCPU;
int clienteUMVCPU;
PCB* pcbAUtilizar;
int quantum;
t_log* logger;
AnSISOP_funciones* funcionesPrimitivas;
AnSISOP_kernel* funcionesPrimitivasKernel;
int quantumRestante;
int finalizarPorSenal=0;

void envioSenal1(){

	finalizarPorSenal=1;

}

void envioSenal(){

	char* pcbSerializado = serializarPCB_con_digito(5,pcbAUtilizar);
	int tamanio=13 * sizeof(u_int32_t);
	send(clienteKernelCPU,pcbSerializado,tamanio,0);
	exit(1);

}

void *planConectar (){
	socket_header handshake;

		handshake.size = sizeof(socket_header);
		handshake.code = 'c'; //CPU



		if( send(clienteUMVCPU, &handshake, sizeof(socket_header), 0) <= 0 )
		{
			log_error(logger, "No se puedo enviar Handshake a la UMV");
		}

	return 0;
}


int main (void){
	char* path = "archivoConfiguracionCpu.conf";
	Puerto* arch = crear_estructura_config(path);
	funcionesPrimitivas = ANSISOP_funciones_a_utilizar();
	funcionesPrimitivasKernel = ANSISOP_funciones_a_utilizar_kernel();

	clienteUMVCPU = crearSocketCliente(arch->ipU, arch->puertoU);
	clienteKernelCPU = crearSocketCliente(arch->ipK, arch->puertoK);

	char* archLog = "mi_log_cpu";
	logger = log_create("CPU.c",archLog,1,LOG_LEVEL_INFO);

	pthread_t hiloUMV;
		if (pthread_create(&hiloUMV, NULL, planConectar, NULL)) {
			log_error(logger, "Error al crear el hilo de ejecucion");
			abort();
		}

	pthread_join(hiloUMV,NULL);



	if( signal(SIGUSR1, envioSenal1) == SIG_ERR ) {
				log_error(logger,"Error con la señal SIGURS1");
	}

	if( signal(SIGINT, envioSenal) == SIG_ERR ) {
				log_error(logger,"Error con la señal SIGINT");
	}

	pthread_t hiloEjecucion;
	if (pthread_create(&hiloEjecucion, NULL, ejecutarCPU, NULL)) {
		log_error(logger, "Error al crear el hilo de ejecucion");
		abort();
	}

	pthread_join(hiloEjecucion,NULL);

	return 0;

}
