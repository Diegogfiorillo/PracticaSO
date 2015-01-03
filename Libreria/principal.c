#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/select.h>

//Funcion para crear un socket servidor
int crearSocketServidor(char* puertoCliente)
{

  struct addrinfo hints;
  struct addrinfo *serverInfo;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // No importa si uso IPv4 o IPv6
  hints.ai_flags = AI_PASSIVE;        // Asigna el address del localhost: 127.0.0.1
  hints.ai_socktype = SOCK_STREAM;    // Indica que usaremos el protocolo TCP

  getaddrinfo(NULL, puertoCliente, &hints, &serverInfo);

  int listenningSocket;
  listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
  if(listenningSocket==-1)
  {
      printf("Fallo en el socket, se espera 1 minuto y se vuelve a intentar");
      sleep(60);
      listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

  }
  bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
  freeaddrinfo(serverInfo);
  return listenningSocket;
}

//Funcion para crear un socket cliente
int crearSocketCliente(char* ipServidor, char* puertoServidor)
{
  struct addrinfo hints;
  struct addrinfo *ServerInfo;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
  hints.ai_socktype = SOCK_STREAM;    // Indica que usaremos el protocolo TCP

  getaddrinfo(ipServidor, puertoServidor, &hints, &ServerInfo);   // Carga en serverInfo los datos de la conexion

  int serverSocket;
  serverSocket = socket(ServerInfo->ai_family, ServerInfo->ai_socktype, ServerInfo->ai_protocol);
  if(serverSocket==-1)
  {
      printf("Fallo en el socket, se espera 1 minuto y se vuelve a intentar");
      sleep(6);
      serverSocket = socket(ServerInfo->ai_family, ServerInfo->ai_socktype, ServerInfo->ai_protocol);
  }
  connect(serverSocket, ServerInfo->ai_addr, ServerInfo->ai_addrlen);
  freeaddrinfo(ServerInfo);   // No lo necesitamos mas

  return serverSocket;
}

//Crear un archivo de log
char* archLog = "mi_log";
logger = log_create("Nombre.c",archLog,1,LOG_LEVEL_INFO);

//Crear semaforo
sem_t nombre_sem;  //Esto va arriba en la definicion global
    int semCreado = sem_init(&nombre_sem,1,1);
    if(semCreado==-1)
    {
        log_error(logger,"No pudo crearse el semaforo");
    }

//Crear estructura del hilo requerido
    t_pthreadNombre* estructuraHilo = malloc(sizeof(t_pthreadNombre));
    estructuraHilo->campo1 = elemento1;
    estructuraHilo->campoN = elementoN;

//Invoco hilo
pthread_t mythreadNombre;
        if (pthread_create(&mythreadNombre, NULL, thread_nombre, estructuraHilo)) {
            log_error(logger,"Error al crear el hilo");
            abort();
        }
 
 pthread_join(mythreadNombre,NULL);       //Si es necesario esperar que termine su ejecucion
 
 printf(
            "    ╔═════════════════════════╗ \n    ║Saludos,Federico y Diego.║ \n    ╚═════════════════════════╝");
    //Porque era hermoso
    

//Serializar!
char* serializar_datos(char* codigoASerializar){
 
    char *paqueteSerializado = malloc(strlen(codigoASerializar));    //Malloc del tamaño a guardar
 
    int offset = 0;
    int size_to_send;
    
    size_to_send = sizeof(u_int32_t);
    memcpy(paqueteSerializado+offset, strlen(codigoASerializar),size_to_send);
    offset +=size_to_send;
 
    size_to_send = strlen(codigoASerializar);
    memcpy(paqueteSerializado+offset, codigoASerializar, size_to_send);
    offset+= size_to_send;
 
    printf(" %s\n", paqueteSerializado); //Chequeamos que este todo ok
 
    return paqueteSerializado;
}

//Deserializar
package* deserializar_paquete(char* paqueteSerializado)
{
 
    u_int32_t size;
    int offset = 0;
    package* paquete = malloc(sizeof(package));
     memcpy(&size,paqueteSerializado,sizeof(u_int32_t));
    paquete->size = size;
    offset+=sizeof(u_int32_t);
    memcpy(paquete->codigo,paqueteSerializado+offset,paquete->size);
    return paquete;
}


//Sendall
int sendall(int s, char* buffer, int* len)          //me aseguro que se envie toda la informacion
{
    int total = 0;
    int bytesleft = *len;
    int n;
    while(total<*len)
    {
        n = send(s, buffer+total, bytesleft, 0);
        if(n==-1) break;
        total+=n;
        bytesleft-=n;
    }
    *len = total;
    return n==-1?-1:0;
}
