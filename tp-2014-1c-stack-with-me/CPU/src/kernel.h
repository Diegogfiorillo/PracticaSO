#ifndef KERNEL_H_
#define KERNEL_H_

typedef struct
{
	unsigned char code;
	size_t size;
} __attribute__((packed)) socket_header  ;

typedef struct estructura_pcb					//estructura que creo con los datos que devuelve el parser
{
	u_int32_t processID;
	u_int32_t progCounter;
	u_int32_t stackSegment;
	u_int32_t codSegment;
	u_int32_t etiq_Segment;
	u_int32_t cursor_stack;
	u_int32_t cantVariables;
	u_int32_t codIndex;
	u_int32_t tam_indice_etiquetas;
	u_int32_t peso;
	u_int32_t socketPrograma;
	u_int32_t prioridad;
}PCB;

char* serializarPCB_con_digito(u_int32_t, PCB*);

#endif /* KERNEL_H_ */
