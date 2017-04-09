/*
 ============================================================================
 Name        : podekex-cliente.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "fuse-basic.h"


int main(int argc, char *argv[])
{

	cliente_osada_create();
	cliente_osada_conectate();
	iniciar_log();
	return fuse_main(argc, argv, &osada_operations, NULL);
}
