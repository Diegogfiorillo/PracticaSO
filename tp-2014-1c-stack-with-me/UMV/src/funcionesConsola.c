


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/collections/list.h"
#include "funcionesConsola.h"
#include "gestionDeSegmentos.h"
#include "commons/log.h"
#include <sys/types.h>
#include <semaphore.h>

extern char* codAlg;
extern char* base;
//extern char* memLogica;
extern int retardo;
extern t_list* listMP;
extern int id_seg;
extern int size_mp;
extern u_int32_t disponible;

extern sem_t sem_disponible;
extern sem_t sem_memLogica;
extern sem_t sem_listMP;
extern sem_t sem_idseg;


//Funciones de la Consola


void mostrarBytePorBytePorArchivo(int cantidad, char * mem, FILE* archivo)
{
	fprintf(archivo,"\nEl contenido de la memoria principal en el lugar solicitado es: \n");
	printf("\nEl contenido de la memoria principal en el lugar solicitado es: \n");
	u_int32_t i;
	for (i = 0; i < cantidad; i++)
	{
		if (mem[i] == '\n' || mem[i] == '\t' || mem[i] == NULL )
		{
			fprintf(archivo," ");
			printf(" ");
		}
		else
		{
			fprintf(archivo,"%c", mem[i]);
			printf("%c", mem[i]);
		}
	}
	fprintf(archivo,"\n");
	printf("\n");
	fclose(archivo);
}

void mostrarContenidoPorArchivo(int base,int offset, int tamanio, char* baseTotal, FILE* archivo)
{
	u_int32_t diferencia = base + offset;
	char* mem = baseTotal+diferencia;
	mostrarBytePorBytePorArchivo(tamanio,mem,archivo);
}

void mostrarBytePorByte(int cantidad, char * mem)
{
	printf("\nEl contenido de la memoria principal en el lugar solicitado es: \n");
	u_int32_t i;
	for (i = 0; i < cantidad; i++)
	{
		if (mem[i] == '\n' || mem[i] == '\t' || mem[i] == NULL )
		{
			printf(" ");
		}
		else
		{
			printf("%c", mem[i]);
		}
	}
	printf("\n");
}

void mostrarContenido(int base,int offset, int tamanio, char* baseTotal)
{
	u_int32_t diferencia = base + offset;
	char* mem = baseTotal+diferencia;
	mostrarBytePorByte(tamanio,mem);
}

void mostrarTablasDeTodos(FILE* archivoDump, char deseaArchivo[])
{
	sem_wait(&sem_listMP);
	if (!strcasecmp(deseaArchivo,"Si"))
	{
		archivoDump= fopen("dump.txt","r+w");
		fprintf(archivoDump,"Todos los segmentos que se encuentran en la memoria son: \n\n");
		printf("Todos los segmentos que se encuentran en la memoria son: \n\n");
		list_sort(listMP,&ordenarPorProceso);

		int index=0;
		int longitud_lista=list_size(listMP);

		fprintf(archivoDump,"SegID: \tLongitud: \tDirFisica: \tDirLogica: \tID_Proc: \n");
		printf("SegID: \tLongitud: \tDirFisica: \tDirLogica: \tID_Proc: \n");

		while(index<longitud_lista)
		{
			memP* segmentos=list_get(listMP,index);
			fprintf(archivoDump,"  %d\t  %d\t\t%p\t   %d\t\t   %d\n",
					segmentos->id_seg,segmentos->longitud,segmentos->memFisica,
					segmentos->dir_logica,segmentos->id_proceso);
			printf("  %d\t  %d\t\t%p\t   %d\t\t   %d\n",
										segmentos->id_seg,segmentos->longitud,segmentos->memFisica,
										segmentos->dir_logica,segmentos->id_proceso);
			index++;
		}//while
		fclose(archivoDump);
	}
	else
	{
		if (!strcasecmp(deseaArchivo,"No"))
		{
			printf("Todos los segmentos que se encuentran en la memoria son: \n\n");
			printf("SegID: \tLongitud: \tDirFisica: \tDirLogica: \tID_Proc: \n");
			list_sort(listMP,&ordenarPorProceso);

			int index=0;
			int longitud_lista=list_size(listMP);
			while(index<longitud_lista)
			{
				memP* segmentos=list_get(listMP,index);
				printf("  %d\t  %d\t\t%p\t   %d\t\t   %d\n",
						segmentos->id_seg,segmentos->longitud,segmentos->memFisica,
						segmentos->dir_logica,segmentos->id_proceso);
				index++;
			}
		}
	}//else
	sem_post(&sem_listMP);
}

//OPERACION
//solicitar posicion de memoria
/* esta funcion lo que hace es devolver la informacion que se encuentra en esa posicion de memoria deseada*/

void funcionOperacionSM()
{

	int tamanio;
	int baseConsola;
	int offset;
	int idProceso;
	FILE* archivoOperacionSM=fopen("archivoOperacionSM.txt","w");
	printf("La tabla de segmentos completa es la siguiente:\n");
	mostrarTablasDeTodos(archivoOperacionSM,"no");

	printf("\n Solicite el contenido de la posicion deseada: \n");
	printf("Ingrese el id de proceso: \n");
	scanf("%d", &idProceso);
	printf("Ingrese la base logica correspondiente al segmento elegido: \n");
	scanf("%d", &baseConsola);
	printf("Ingrese el offset: \n");
	scanf("%d", &offset);
	printf("Ingrese su tamaño: \n");
	scanf("%d", &tamanio);

	bool filtrarPorProceso (void* seg)
		{
			memP* segmento = (memP*) seg;
			return segmento->id_proceso == idProceso;
		}

		bool segmentoAnteriorLogica(void * seg1, void * seg2)
		{
			memP* segmento1=(memP*) seg1;
			memP* segmento2=(memP*) seg2;

			return segmento1->dir_logica < segmento2->dir_logica;
		}

	printf("Desea guardar el contenido de las posiciones en un archivo? Si/No\n");
	char respuesta[2];
	scanf("%s",respuesta);
	while(strcasecmp(respuesta,"Si")!=0 && strcasecmp(respuesta,"No")!=0)
	{
		printf("La opcion no es correcta. Por favor, escriba Si/No para generar un archivo\n");
		printf("¿Desea generar archivo en disco? (Si/No) \n");
		scanf("%s", respuesta);
	}

	printf("llega aca \n");
		t_list* listaDePrograma=list_filter(listMP,filtrarPorProceso);
		list_sort(listaDePrograma,segmentoAnteriorLogica);
		printf("llega a ordenar la lista \n");
		int i=0;
		printf("justo antes del for el tam de la lista es %d \n",list_size(listaDePrograma) );
		for(i=0; i<list_size(listaDePrograma); i++)
		{
			printf("entra al for \n");
			memP* segmento=list_get(listaDePrograma,i);
			if(segmento->dir_logica==baseConsola)
			{
				printf("El segmento ingresado coincide \n");
				//chequeamos que no se pase lo que piden
				if((baseConsola+offset+tamanio)<(segmento->dir_logica+segmento->longitud))
				{
					//mostramos:

					if(strcasecmp(respuesta,"No")==0)
					{
						printf("Eligio no guardar un archivo \n");
						mostrarContenido(baseConsola,offset,tamanio,base);
					}
					else
					{
						mostrarContenidoPorArchivo(baseConsola,offset,tamanio,base,archivoOperacionSM);
					}

				}
				else
				{
					printf("Los datos ingresados exceden los limites del segmento elegido \n");
				}
			}
			else
			{
				if(i+1==list_size(listaDePrograma))
				{
					printf("No ha ingresado una base correcta \n");
				}
			}
		} //del for

		list_destroy_and_destroy_elements(listaDePrograma,free);
}

//crear buffer
void funcionOperacionEB() //ya esta
{
	FILE* archivoBuffer;
	int baseConsola=0;
	int offset=0;
	int tamanio;
	char respuesta[3];
	respuesta[2]='\0';
	printf("Ingrese el tamanio, y a continuacion la cadena a escribir, que debe ser igual o menor al tamanio ingresado: \n");
	scanf(" %d \n", &tamanio);
	char buffer[tamanio];
	fgets(buffer,tamanio+1,stdin);
	printf("La cadena leida es: %s \n", buffer);

	printf("Ingrese base\n");
	scanf("%d", &baseConsola);
	printf("Ingrese offset\n");
	scanf("%d", &offset);

	printf("Desea guardar el contenido del buffer en un archivo? Si/No ");
	scanf("%s", respuesta);
	while(strcasecmp(respuesta,"Si")!=0 && strcasecmp(respuesta,"No")!=0)
	{
		printf("La opcion ingresada no es correcta. Por favor indique si desea el archivo.\n");
		scanf("%s", respuesta);
	}

	t_list* listaVacios=list_create();
	list_sort(listMP,&segmentoAnterior);
	sem_wait(&sem_listMP);
	listaVacios = listaDeVacios(listMP);

	int longLista=list_size(listaVacios);
	if (longLista==0)
	{
		printf("No se puede crear el buffer porque no hay memoria libre\n");
	}
	else
	{
		int siGuardo=0;
		int w=0;
		for (w=0; w<longLista; w++)
		{
			vacios* vacio=list_get(listaVacios,w);
			int dondeGuardar=baseConsola+offset;
			if(dondeGuardar>=vacio->fisica-base)  //si dondeGuardar es mayor o igual al primer espacio libre
			{
				if(tamanio<=(vacio->fisica+vacio->longitud)-base) //si entra en el espacio vacio
				{
					memcpy(base+dondeGuardar,buffer,tamanio);
					w=longLista;
					siGuardo=1;
					printf("Buffer creado con exito en la posicion %p (De baseIngresada+Offset %d)\n",base+dondeGuardar, dondeGuardar);
					printf("Contenido: %s\n",buffer);
					if(strcasecmp(respuesta,"Si")==0)
					{
						archivoBuffer=fopen("archivoBuffer.txt","r+w");
						fprintf(archivoBuffer,"\nBuffer creado con exito en la posicion %p (De baseIngresada+Offset %d)\n",base+dondeGuardar, dondeGuardar);
						fprintf(archivoBuffer,"Contenido: %s" ,buffer);
						fprintf(archivoBuffer,"\n");
						fclose(archivoBuffer);
					}

				}
			}
		}
		if(siGuardo==0)
		{
			printf("No se puede crear el buffer en una posicion de memoria ocupada \n");
		}

	} //del if del lugar
	sem_post(&sem_listMP);
}


int crearSegmentoDeConsola(int size, int id_proc, char info[])
{

info[size+1]='\0';
	if (size>0)
	{
	if (size <= disponible)
	{
		int index=0;
		int lista_size=list_size(listMP);
		memP* segmentoCreado;
		while(index<lista_size ) //si la lista NO esta vacia
		{
			if (strcmp(codAlg,"FF")==0) //AGREGO CON FIRST-FIT
			{
				segmentoCreado = agregar_segun_FF(size,id_proc, info);
			}
			else //AGREGO CON WORST-FIT
			{
				segmentoCreado = agregar_segun_WF(id_proc,info,size);
			}
			//actualizo tamaño total de memoria principal

			sem_wait(&sem_idseg);
			id_seg++;
			sem_post(&sem_idseg);

			return segmentoCreado->dir_logica;

		} //del while
		segmentoCreado = crear_primer_nodo(size,id_proc, info);
		sem_wait(&sem_idseg);
		id_seg++;
		sem_post(&sem_idseg);
		return segmentoCreado->dir_logica;

	}//del segundo if
	else
	{
		printf("Memory Overload \nEl segmento a guardar no entra en memoria.\n");  //no entra en memoria
		return -1;
	}
	}
	else
	{
		printf("No se puede guardar un segmento de tamanio negativo o igual a cero.\n");
		return -1;
	}

	return EXIT_SUCCESS;
}


void funcionOperacionC(t_log* logger)
{
	int tamanio;
	int id_proc;
	int segmento;
	printf("Ingrese el tamanio del segmento a crear, y a continuacion la informacion: \n");
		scanf(" %d \n", &tamanio);
		char info[tamanio];
		fgets(info,tamanio+1,stdin);

	printf("Ingrese el id del proceso al que pertenece el segmento a crear:\n");
	scanf("%d", &id_proc);

	if(tamanio>0)
	{
		segmento=crearSegmentoDeConsola(tamanio,id_proc, info);
		if(segmento==-1)
		{
			log_info(logger,"Memory Overload al intentar crear segmento\n");
			printf("Memory Overload - El segmento que intenta grabar no cabe en memoria\n");
		}
		else
		{

			log_info(logger,"Segmento creado\n");
			printf("Se ha creado el segmento en la siguiente direccion %d \n", segmento);
		}
	}
	else
	{
		log_info(logger,"No se puede crear un segmento negativo\n");
		printf("No se puede crear un segmento de tamaño negativo\n");
	}
	log_info(logger,"Finalizada la operacion crear segmento\n");
}


void funcionOperacionD(t_log* logger)
{
	int identificador;
	int proceso=0;
	int info=0;
	printf("Ingrese identificador de segmento \n");
	scanf("%d", &identificador);
	printf("Ingrese identificador del programa \n");
	scanf("%d", &proceso);

	//semaforo
	info = destruirSegmento(identificador, proceso);
	//semaforo

	if(info==0)
	{
		log_info(logger,"El segmento que intenta destruir no existe");
		printf("El segmento perteneciente al identificador del programa %d del proceso %d no existe\n", identificador,proceso);
	}
	else
	{
		log_info(logger,"Segmento destruido con exito");
		printf("Se a destruido con exito el segmento %d del proceso %d \n",identificador,proceso);
	}
	log_info(logger,"Finalizada la operacion destruir segmento");
}

void funcionOperacion (t_log* logger)
{
	int clave=0;
	while(clave==0)
	{
		int operacionElegida;
		printf("-Seleccione la operacion que desea realizar,no se pueden ingresar caracteres, solo números\n");
		printf("-Solicitar posición de memoria (1)\n");
		printf("-Escribir buffer por teclado(2)\n");
		printf("-Crear segmento(3)\n");
		printf("-Destruir segmento(4)\n");
		printf("-Volver al menu anterior(5)\n");
		scanf("%d", &operacionElegida);

		switch(operacionElegida)
		{
		case 1: {
			funcionOperacionSM();
			clave=1;
			break; //fin caso solicitar memoria
		}
		case 2: {
			funcionOperacionEB();
			clave=1;
			break; //fin caso escribir memoria
		}
		case 3: {
			funcionOperacionC(logger);
			clave=1;
			break; //fin caso crear segmento
		}
		case 4: {
			funcionOperacionD(logger);
			clave=1;
			break; //fin caso destruir segmento
		}
		case 5:
		{
			clave=1;
			break;
		}
		default: {
			printf("La opcion ingresada no corresponde\n");
			break;
		}
		}//fin switch operacion
	}//del while
}


//RETARDO ya esta
void funcionRetardo (t_log* logger)
{
	int retardoIngresado;
	printf("Ingrese el retardo que desea que tenga el sistema.\n");
	scanf("%d", &retardoIngresado);
	if(retardoIngresado>=0)
	{
		retardo=retardoIngresado;
		log_info(logger,"Retardo Cambiado");
	}
	else
	{
		log_info(logger,"Intento asignar un retardo negativo");
		printf("No se puede asignar un retardo menor a cero");
	}
	log_info(logger,"Finalizado el cambio de retardo.");
}

//CAMBIAR ALGORITMO   ya esta
void funcionCambiarAlgoritmo(t_log* logger)
{
	int algoritmoElegido;
	int clave=0;
	while(clave==0)
	{
		printf("Ingrese el algoritmo que desea implementar\n");
		printf("Algoritmo First-Fit (1)\n");
		printf("Algoritmo Worst-Fit (2)\n");
		scanf("%d", &algoritmoElegido);
		if(algoritmoElegido==1)
		{
			codAlg = "FF";
			clave=1;
			printf("Se ha cambiado el algoritmo a FF\n");

		}else
		{
			if (algoritmoElegido==2){
				codAlg= "WF";
				clave=1;
				printf("Se ha cambiado el algoritmo a WF\n");

			}else
			{
				log_info(logger,"Ingreso un algoritmo erroneo");
				printf("No ha ingresado correctamente el algoritmo que desea implementar\n");
			}

		}
	}
}

//COMPACTAR ya esta
void funcionCompactar(t_log* logger)
{
	char* devolucion = funcionCompactarAchetada();

	if (!strcasecmp(devolucion,"null"))
	{
		log_info(logger,"La memoria esta llena y no se puede compactar\n");
		printf("La memoria esta llena y no se puede compactar\n");
	}
	else
	{
		if(!strcasecmp(devolucion,"empt"))
		{
			log_info(logger,"La memoria esta vacia y no hay nada para compactar\n");
			printf("La memoria esta vacia y no hay nada para compactar\n");
		}
		else
		{
			printf("Finalizo la funcion de compactar la memoria\n");
			log_info(logger,"Finalizo la funcion de compactar la memoria\n");
		}
	}
}










void funcionDumpSESinArch ()
{
	sem_wait(&sem_listMP);
	list_sort(listMP,&segmentoAnterior);
	int longLista=list_size(listMP);
	if(longLista==0)
	{
		printf("0-%d Memoria Libre \n", size_mp-1);
	}
	else
	{
		int index=0;
		while (index<longLista)
		{
			memP* seg=(memP*)list_get(listMP,index);
			if(seg->memFisica!=base && index==0)
			{
				printf("0-%d: Memoria Libre \n", seg->memFisica-base-1);
				//printf("%d-%d: Segmento %d, Proceso %d \n", seg->dir_logica, seg->dir_logica+seg->longitud-1, seg->id_seg, seg->id_proceso);
			}
			memP* seg2;
			if(index+1<longLista)
			{
				seg2=(memP*)list_get(listMP,index+1);

				if(seg->memFisica+seg->longitud==seg2->memFisica)
				{
					printf("%d-%d: Segmento %d, Proceso %d \n",seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
				}
				else
				{
					printf("%d-%d: Segmento %d, Proceso %d \n", seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
					printf("%d-%d: Memoria Libre \n", seg->memFisica-base+seg->longitud, seg2->memFisica-base-1);
				}
			}
			else
			{
				printf("%d-%d: Segmento %d, Proceso %d \n", seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
			}
			index++;
		}
		memP* seg=(memP*)list_get(listMP,longLista-1);
		if(seg->memFisica+seg->longitud!=base)
		{
			printf("%d-%d: Memoria Libre\n",seg->memFisica+seg->longitud-base,size_mp-1);
		}
	}
	sem_post(&sem_listMP);
}


void funcionDumpSEConArch (FILE* archivoDump)
{
	archivoDump= fopen("dump.txt","w");
	fprintf(archivoDump,"\n");
	printf("\n");
	sem_wait(&sem_listMP);
	list_sort(listMP,&segmentoAnterior);
	int longLista=list_size(listMP);
	if(longLista==0)
	{
		fprintf(archivoDump,"0-%d Memoria Libre \n", size_mp-1);
		printf("0-%d Memoria Libre \n", size_mp-1);
	}
	else
	{
		int index=0;
		while (index<longLista)
		{
			memP* seg=(memP*)list_get(listMP,index);
			if(seg->memFisica!=base && index==0)
			{
				fprintf(archivoDump,"0-%d: Memoria Libre \n", seg->memFisica-base-1);
				printf("0-%d: Memoria Libre \n", seg->memFisica-base-1);
				//		printf("%d-%d: Segmento %d, Proceso %d \n", seg->dir_logica, seg->dir_logica+seg->longitud-1, seg->id_seg, seg->id_proceso);
			}
			memP* seg2;
			if(index+1<longLista)
			{
				seg2=(memP*)list_get(listMP,index+1);

				if(seg->memFisica+seg->longitud==seg2->memFisica)
				{
					fprintf(archivoDump,"%d-%d: Segmento %d, Proceso %d \n",seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
					printf("%d-%d: Segmento %d, Proceso %d \n",seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
				}
				else
				{
					fprintf(archivoDump,"%d-%d: Segmento %d, Proceso %d \n", seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
					fprintf(archivoDump,"%d-%d: Memoria Libre \n", seg->memFisica-base+seg->longitud, seg2->memFisica-base-1);

					printf("%d-%d: Segmento %d, Proceso %d \n", seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
					printf("%d-%d: Memoria Libre \n", seg->memFisica-base+seg->longitud, seg2->memFisica-base-1);
				}
			}
			else
			{
				fprintf(archivoDump,"%d-%d: Segmento %d, Proceso %d \n", seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
				printf("%d-%d: Segmento %d, Proceso %d \n", seg->memFisica-base, (seg->memFisica+seg->longitud)-base-1, seg->id_seg, seg->id_proceso);
			}
			index++;
		}
		memP* seg=(memP*)list_get(listMP,longLista-1);
		if(seg->memFisica+seg->longitud!=base)
		{
			fprintf(archivoDump,"%d-%d: Memoria Libre\n",seg->memFisica+seg->longitud-base,size_mp-1);
			printf("%d-%d: Memoria Libre\n",seg->memFisica+seg->longitud-base,size_mp-1);
		}
	}

	fprintf(archivoDump,"\n");
	printf("\n");
	fclose(archivoDump);
	sem_post(&sem_listMP);
}

void funcionDumpCONT(FILE* archivoDump, char respuesta[])
{
	int offset, tamanio;
	printf("\n Solicite el contenido de la posicion deseada: \n");
	printf("Ingrese el offset\n");
	scanf("%d", &offset);
	printf("Ingrese el tamanio \n");
	scanf("%d", &tamanio);

	sem_wait(&sem_listMP);
	if( size_mp < (tamanio + offset))
	{
		if(!strcasecmp(respuesta,"Si"))
		{
			archivoDump= fopen("dump.txt","r+w");
			fprintf(archivoDump,"Segmentation Fault. Excede la memoria principal\n");
			printf("Segmentation Fault. Excede la memoria principal\n");
			fclose(archivoDump);
		}
		else
		{
			if(!strcasecmp(respuesta,"No"))
			{
				printf("Segmentation Fault. Excede la memoria principal\n");
			}
		}
	}
	else
	{
		if(!strcasecmp(respuesta,"No"))
		{
			mostrarContenido(0,offset,tamanio,base);
		}
		else
		{
			if(!strcasecmp(respuesta,"Si"))
			{
				archivoDump=fopen("dump.txt","r+w");
				mostrarContenidoPorArchivo(0,offset,tamanio,base,archivoDump);
			}
		}//Del else chiqiuto
	}//else grande
	sem_post(&sem_listMP);
}

void imprimirSegmentosPorProceso(int id_proc, char deseaArchivo[], FILE* archivoDump)
{
	sem_wait(&sem_listMP);
	list_sort(listMP,&segmentoAnterior);
	int index=0;
	int longitud_lista=list_size(listMP);

	if (!strcasecmp(deseaArchivo,"Si"))
	{
		archivoDump= fopen("dump.txt","r+w");
		fprintf(archivoDump,"Todos los segmentos de dicho proceso son: \n\n");
		printf("Todos los segmentos de dicho proceso son: \n\n");

		fprintf(archivoDump,"SegID: \tLongitud: \tDirFisica: \tDirLogica: \tID_Proc: \n");
		printf("SegID: \tLongitud: \tDirFisica: \tDirLogica: \tID_Proc: \n");

		while(index<longitud_lista)
		{
			memP* segmentos=list_get(listMP,index);
			if(segmentos->id_proceso==id_proc)
			{
				fprintf(archivoDump,"  %d\t  %d\t\t%p\t   %d\t\t   %d\n",
						segmentos->id_seg,segmentos->longitud,segmentos->memFisica,
						segmentos->dir_logica,segmentos->id_proceso);

				printf("  %d\t  %d\t\t%p\t   %d\t\t   %d\n",
							segmentos->id_seg,segmentos->longitud,segmentos->memFisica,
							segmentos->dir_logica,segmentos->id_proceso);
			}
			index++;
		}

		fclose(archivoDump);
	}//if del archivo
	else
	{
		if (!strcasecmp(deseaArchivo,"No"))
		{
			printf("Todos los segmentos de dicho proceso son: \n\n");
			printf("SegID: \tLongitud: \tDirFisica: \tDirLogica: \tID_Proc: \n");

			while(index<longitud_lista)
			{
				memP* segmentos=list_get(listMP,index);
				if(segmentos->id_proceso==id_proc)
				{
					printf("  %d\t  %d\t\t%p\t   %d\t\t   %d\n",
							segmentos->id_seg,segmentos->longitud,segmentos->memFisica,
							segmentos->dir_logica,segmentos->id_proceso);
				}
				index++;
			}//(del while))
		}//del if
	}//del else
	sem_post(&sem_listMP);
}

bool ordenarPorProceso (void* seg1, void* seg2)
{
	memP* segmento1 = (memP*) seg1;
	memP* segmento2 = (memP*) seg2;

	return segmento1->id_proceso < segmento2->id_proceso;
}


void funcionDumpTS(FILE* archivoDump, char deseaArchivo[])
{
	int segmentos;
	int identificadorDelProceso;
	int claveOpcion=0;
	while(claveOpcion==0)
	{
		printf("Desea ver la tablas de segmentos de todos los procesos (1) \n");
		printf("Desea ver la tabla de segmentos de un proceso determinado (2) \n");
		scanf("%d", &segmentos);

		if(segmentos==1)
		{
			mostrarTablasDeTodos(archivoDump,deseaArchivo);
			claveOpcion=1;
		}
		else
		{
			if(segmentos==2)
			{
				printf("Ingrese el identificador del proceso del cual desea obtener los segmentos \n");
				scanf("%d", &identificadorDelProceso);

				imprimirSegmentosPorProceso(identificadorDelProceso, deseaArchivo,archivoDump);

				claveOpcion=1;
			}
			else
			{
				printf("La opcion ingresada no existe\n");
			}
		}
	}//while
}

void funcionDump(FILE* archivoDump)
{
	char deseaArchivo[3];
	deseaArchivo[2]='\0';

	printf("¿Desea generar archivo en disco? (Si/No) \n");
	scanf("%s", deseaArchivo);
	while(strcasecmp(deseaArchivo,"Si")!=0 && strcasecmp(deseaArchivo,"No")!=0)
	{
		printf("La opcion no es correcta. Por favor, escriba Si/No para generar un archivo\n");
		printf("¿Desea generar archivo en disco? (Si/No) \n");
		scanf("%s", deseaArchivo);
	}

	int clave=0;
	while(clave==0)
	{
		int queElige;
		printf("Desea conocer las tablas de segmentos de todos los procesos o de un proceso en particular (1)\n");
		printf("Desea conocer los segmentos de los programas y el espacio libre (2)\n");
		printf("Desea conocer el contenido de la memoria principal (3)\n");
		scanf("%d", &queElige);
		switch(queElige)
		{
		case 1:
		{
			funcionDumpTS(archivoDump,deseaArchivo);
			clave=1;
		}
		break;
		case 2:
		{
			if(!strcasecmp(deseaArchivo,"No"))
			{
				funcionDumpSESinArch(archivoDump);
			}
			else
			{
				funcionDumpSEConArch(archivoDump);
			}
			clave=1;
		}
		break;
		case 3:
		{
			funcionDumpCONT(archivoDump,deseaArchivo);
			clave=1;
		}
		break;
		default:
		{
			printf("No existe la opcion elegida");
		}
		}//fin del case
	}//while
}


