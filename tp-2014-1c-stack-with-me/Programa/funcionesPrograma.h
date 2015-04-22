#ifndef FUNCIONESPROGRAMA_H_
#define FUNCIONESPROGRAMA_H_

#define MAX_MESSAGE_SIZE 300

typedef struct t_arch
{
	char* puerto;
	char* ip;
}config;

typedef struct
{
	uint32_t size;
	char* codigo;
}package;

typedef struct
{
	char* texto;
}sentenciasImprimir;


int enviarCodigo(char*, u_int32_t*);
char* obtenerPrimerLinea(char*);
char* serializar_paquete_programa(package*);
package* deserializar_paquete(char*);
void esperarSentencias();
int recibirYDeserializar(int, char*,sentenciasImprimir*);
void finalizar();

#endif /* FUNCIONESPROGRAMA_H_ */
