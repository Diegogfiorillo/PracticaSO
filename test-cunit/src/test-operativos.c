/*
 * test-operativos.c
 *
 *  Created on: 12/01/2015
 *      Author: federico
 */
#include "test-operativos.h"


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

    //printf(" %s\n", paqueteSerializado); //Chequeamos que este todo ok

    return paqueteSerializado;
}

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

