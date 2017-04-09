/*
 * server.c
 *
 *  Created on: 25/9/2016
 *      Author: utnso
 */
#include "server.h"

t_server* server_create(int puerto, char *ip, int backlog)
{
		t_server *new_server = malloc(sizeof(t_server));

		//Configuro direccion de servidor//
		address_config_in direccionServidor =configurar_address_in(puerto, ip);

		//Creo el socket para el server//
		int server_socket = socket(AF_INET,SOCK_STREAM,0);

		activar_reutilizacion_de_direcciones(1,server_socket);

		//Asocio socket al puerto por donde escuchará//
		server_asociate_a_puerto(server_socket, &direccionServidor);

		new_server->socket_asociado=server_socket;
		new_server->backlog= backlog;
		return new_server;
}

int server_asociate_a_puerto(int server, address_config_in *address)
{
	int resultBind;
	resultBind = bind(server,(void*) address,sizeof(*address));

	if(resultBind !=0)
	{	perror("Falló el bind");
		return 1;
	}
	return resultBind;
}

void server_escucha(t_server *server)
{
	listen(server->socket_asociado,server->backlog);
}

void server_cerra_cliente(int cliente)
{
	close(cliente);
}

int server_acepta_conexion_cliente(t_server *server)
{
	address_config_in direccionCliente;
	unsigned int tamanioDireccion = sizeof(address_config_in);
	int cliente = accept(server->socket_asociado,(void*) &direccionCliente, &tamanioDireccion);
	if(cliente <0)
	{
		return 0;
	}
	return cliente;
}
