/*
 * main.c
 *
 *  Created on: 12/01/2015
 *      Author: federico
 */


#include "CUnit/Basic.h"
#include <unistd.h>
#include "test-operativos.h"

static package* paquete;
static char* codigo = "MiraDiegoSonTest";

int inicializarPaquete() {
	package* paquete = malloc(sizeof(package));
	return paquete != NULL?0:-1;
}

int limpiarPaquete() {
	free(paquete);
	return 0;
}

void test_serializacion()
{
	package* paquete = inicializarPaquete();
	char* cadenaSerializada = serializar_datos(codigo);
	paquete = deserializar_paquete(cadenaSerializada);
	CU_ASSERT_STRING_EQUAL(paquete->codigo,codigo);
	CU_ASSERT_EQUAL(paquete->size,17);
}

void agregar_tests() {
	CU_pSuite archivo = CU_add_suite("Serializacion", inicializarPaquete, limpiarPaquete);
	CU_add_test(archivo,"test_serializacion",test_serializacion);
}


int main() {
   CU_initialize_registry();

   agregar_tests();

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   return CU_get_error();
}

