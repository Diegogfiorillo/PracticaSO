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
