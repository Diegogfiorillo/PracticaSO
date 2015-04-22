//Proceso umv


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "funcionesUMV.h"
#include "funcionesConsola.h"
#include "funcionesCPU.h"

#define ROJO "\x1B[31m"

//Listado de variables globales
char* base;
u_int32_t memLogica;
t_list* listMP;
int retardo=0;
u_int32_t tamanio_stack;
u_int32_t size_mp;
u_int32_t disponible;
char* codAlg;


//Listado de Semaforos
sem_t sem_disponible;
pthread_mutex_t sem_memLogica = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_listMP;
sem_t sem_idseg;

int main(void)
{

	printf("%s", ROJO);
	printf("	      _^__                                      __^__\n            ( ___ )------------------------------------( ___ )   \n             | / |                                      | \\ |  \n             | / |        Bienvenidos a la UMV          | \\ |   \n             |___|                                      |___| \n            (_____)------------------------------------(_____)\n" );


	//inicializamos los semaforos
	sem_init(&sem_disponible,1,1);
	sem_init(&sem_listMP,1,1);
//	sem_init(&sem_memLogica,1,1);
	sem_init(&sem_idseg,1,1);

	char* path = "UMVconfig";
	t_config* configFile = config_create(path);

	size_mp = config_get_int_value(configFile, "MPSIZE");
	disponible = size_mp;
	char* mp = malloc(size_mp);
	base = mp;
	listMP = list_create();
	codAlg=config_get_string_value(configFile, "CODIGOALGORITMO");

	char* archLogUMV = "mi_log_umv";
	char* ConsolaLog = "Log_Consola_UMV";
	t_log* loggerUMV = log_create("logumv.c", archLogUMV,1,LOG_LEVEL_INFO);
	t_log* logConsola = log_create("logconsola.c",ConsolaLog,1,LOG_LEVEL_INFO);
	char* archLogKernel= "mi_log_Kernel";
	t_log* logKernel = log_create("logKernel.c", archLogKernel,1,LOG_LEVEL_INFO);
	char* archLogCPU= "mi_log_CPU";
	t_log* logCPU = log_create("logCpu.c", archLogCPU,1,LOG_LEVEL_INFO);

	if( signal(SIGINT, NULL) == SIG_ERR ) {
				log_error(loggerUMV, "Se dio de baja el programa por parte del usuario");

		}

	hiloConsola* argConsola=malloc(sizeof(hiloConsola));
	argConsola->logConsola = logConsola;
	argConsola->logger = loggerUMV;

	hiloKernelyCPU* argCPU=malloc(sizeof(hiloKernelyCPU));
	argCPU->puerto = config_get_string_value(configFile, "PUERTOCPU");
	argCPU->logger = logCPU;

	hiloKernelyCPU* argKernel=malloc(sizeof(hiloKernelyCPU));
	argKernel->puerto = config_get_string_value(configFile, "PUERTOKERNEL");
	argKernel->logger = logKernel;

	hiloCadaCPU* argCadaCPU=malloc(sizeof(hiloCadaCPU));
	argCadaCPU->logger = logCPU;

	if (mp==NULL)
	{
		log_info(loggerUMV,"No se pudo reservar el espacio en memoria");
	}
	else
	{
		log_info(loggerUMV,"Reservada la memoria principal");
	}
	 pthread_t KernelThread;
	 pthread_t CPUThread;
	 pthread_t ConsoleThread;
	 
	     int  Kth, CPUth;
	     int Cth;

	     Kth = pthread_create( &KernelThread, NULL, Kernel_function, (void*)argKernel);
	     if(Kth!=0)
	     {
	    	 log_info(loggerUMV,"Error al crear el hilo del Kernel");
	         fprintf(stderr,"Error - No se pudo crear el thread del Kernel. Devolvio: %d\n",Kth);
	         exit(EXIT_FAILURE);
	     }

	     Cth = pthread_create( &ConsoleThread, NULL, Console_function, (void*)argConsola);
	     if(Cth!=0)
	     {
	    	 log_info(loggerUMV,"Error al crear el hilo de la Consola");
	    	 fprintf(stderr,"Error - No se pudo crear el thread de la Consola. Devolvio: %d\n",Cth);
	    	 exit(EXIT_FAILURE);
	     }

	     CPUth = pthread_create( &CPUThread, NULL, CPU_function, (void*)argCPU);
	     if(CPUth!=0)
	     {
	    	 log_info(loggerUMV,"Error al crear el hilo de Cpu");
	    	 fprintf(stderr,"Error - No se pudo crear el thread del Cpu. Devolvio: %d\n",CPUth);
	    	 exit(EXIT_FAILURE);
	     }


	     pthread_join( KernelThread, NULL);
	     pthread_join( ConsoleThread, NULL);
	     pthread_join( CPUThread, NULL);


		list_destroy_and_destroy_elements(listMP,free);
	     free(mp);
	     printf( "                       ╔═════════════════════════╗ \n                       ║  Saludos,Mackolou <3.   ║ \n                       ╚═════════════════════════╝" );
	     exit(EXIT_SUCCESS);
}

