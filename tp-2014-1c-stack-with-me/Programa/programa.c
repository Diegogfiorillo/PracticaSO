#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "funcionesPrograma.h"

#define SHEBANG "#!./ProcesoPrograma"
#define MAXFILESIZE 10000
#define PACKAGESIZE 1024
#define AZUL "\x1B[36m"

int kernelSocket;
t_log* logger;

int main(int argc, char** argv) {
	printf("%s", AZUL);
	if(argc!=2)
	{
		printf("No se pudo ejecutar el proceso programa. Asegurese de llamarlo correctamente\n");
		return EXIT_SUCCESS;
	}
	char* archLog = "mi_log";

	logger = log_create("PruebaPrograma.c",archLog,1,LOG_LEVEL_INFO);

	if (signal(SIGINT, finalizar) == SIG_ERR)
	log_error(logger, "No se puede leer la señal\n");

	char* nomArch = argv[1];

	FILE* script = fopen(nomArch, "r");
	char* codigo = malloc(MAXFILESIZE);
	fread(codigo, MAXFILESIZE, 100, script);

	char* sheBang = malloc(20);
	char* copia = strdup(codigo);
	char* codigoPrograma = malloc(strlen(codigo));
	if (string_starts_with(copia, SHEBANG))
	{
		sheBang = obtenerPrimerLinea(copia);
		int pos = strlen(sheBang);
		memcpy(codigoPrograma, codigo + pos + 1, strlen(codigo) - pos);

		free(sheBang);

		free(codigo);

		package* paquete = malloc(sizeof(package));
		paquete->size = strlen(codigoPrograma);
		paquete->codigo = codigoPrograma;
		free(codigoPrograma);
		//serializo el codigo con su longitud
		char* codigoSerializado = serializar_paquete_programa(paquete);

		t_config* archivo = config_create("PuertoKernel.conf");
		config* archivoConfig = malloc(sizeof(config));
		archivoConfig->ip = config_get_string_value(archivo, "IP");
		archivoConfig->puerto = config_get_string_value(archivo, "PUERTO");

		struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
		hints.ai_socktype = SOCK_STREAM;// Indica que usaremos el protocolo TCP

		getaddrinfo(archivoConfig->ip, archivoConfig->puerto, &hints,
				&serverInfo);	// Carga en serverInfo los datos de la conexion


		kernelSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
				serverInfo->ai_protocol);

		connect(kernelSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
		freeaddrinfo(serverInfo);	// No lo necesitamos mas

		printf("Conectado al servidor.\n");
		printf(
				"	      _^__                                      __^__\n            ( ___ )------------------------------------( ___ )   \n             | / |                                      | \\ |  \n             | / |    Bienvenidos al proceso Kernel     | \\ |   \n             |___|                                      |___| \n            (_____)------------------------------------(_____) \n");


		if (send(kernelSocket, codigoSerializado, paquete->size+sizeof(u_int32_t),
				0) == -1) {
			perror("Envio al Kernel ha resultado mal: ");
		} else {
			perror("Envio al Kernel ha resultado bien: ");
		}



		 esperarSentencias();

		printf("Programa Desconectado.\n");

		free(paquete);
		close(kernelSocket);
		fclose(script);

	} else {
		printf("                       $¶     ¶     ¶¢\n	           ¶¶¶¶¶¶¶       ¶¢   ¶   ø¶\n	          ¶¶    ø¶¶¶      oø  ø  øo\n	          ¶7       ¶¶¶      1   1    1o\n	       ¶¶¶¶¶¶¶       ¶¶¶7        1o¶¶¶ø\n	       ¶¶¶¶¶¶¶         ¶¶¶¶¶¶¶¶  1\n	     o¶¶¶¶¶¶¶¶¶ø                  o$¢\n	   ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶          ¢  1ø   1¶¶o\n	  ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶o       1$   ¶\n	 ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶       ¶    o¶´\n	 ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶     ¶¶\n	 ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶\n	 ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶\n	  ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶ \n	   ¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶ \n	   1¶¶¶¶¶¶¶¶¶¶¶¶¶¶ \n	       ¶¶¶¶¶¶¶¶\n");
		log_error(logger, "El script no empieza con la sheBang: %s", SHEBANG);

	}
	return EXIT_SUCCESS;
}
