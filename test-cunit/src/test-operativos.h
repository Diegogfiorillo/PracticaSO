/*
 * test-operativos.h
 *
 *  Created on: 12/01/2015
 *      Author: federico
 */

#ifndef ARCHIVO_H_
#define ARCHIVO_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

typedef struct
{
	int size;
	char* codigo;

}package;

char* serializar_datos(char*);
package* deserializar_paquete(char*);


#endif /* ARCHIVO_H_ */
