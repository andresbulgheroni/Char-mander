/*
 ============================================================================
 Name        : pokedex-server.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "osada.h"
#include "file_manipuling.h"
#include "ls_and_atributes.h"
#include "osada_generales.h"
#include "borrados.h"
#include "comunication.h"
#include "semaphore.h"

sem_t semaforo_terminacion;

extern t_disco_osada* disco;

int main(int argc, char* argv[])
{
	char* nombre_disco = string_new();
	string_append(&nombre_disco,argv[1]);
	char* nombre_posta = array_last_element(nombre_disco);
	free(nombre_disco);
	printf("Abriendo disco %s ...\n",nombre_posta);
	free(nombre_posta);
	disco = osada_disco_abrite(argv[1]);
	printf("Iniciando semaforos...\n");
	iniciar_semaforos();
	printf("Recuperando estructuras basicas...\n");
	disco->cantidad_archivos_libres=disco_recupera_cantidad_Archivos();
	disco->cantidad_bloques_libres=disco_recupera_cantidad_bloques_libres();
	disco_recupera_tus_archivos();
	recuperar_tabla_de_asignaciones();
	ejecutar_servidor();
	return EXIT_SUCCESS;
}
