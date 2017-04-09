/*
 ============================================================================
 Name        : mapa.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "map-commons.h"

int main(int argc, char *argv[])
{
	if(argv[1]==NULL || argv[2]==NULL)
	{
		printf("Necesito el nombre del mapa y la ruta pokedex!\n");
		exit(1);
	}
	else
	{
		ejecutar_mapa(argv[1],argv[2]);
			return EXIT_SUCCESS;
	}

}

