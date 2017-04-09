/*
 * cliente.c
 *
 *  Created on: 25/9/2016
 *      Author: utnso
 */

#include "cliente.h"

int cliente_create(int puerto, char *ip)
{
	address_config_in direccionServidor = configurar_address_in(puerto, ip);

	int server = socket(AF_INET,SOCK_STREAM,0);

	int resultConect = connect(server, (void*) &direccionServidor,sizeof(direccionServidor));
	if(resultConect !=0)
		{perror("Error de conexión");}
	return server;
}

