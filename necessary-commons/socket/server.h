/*
 * server.h
 *
 *  Created on: 25/9/2016
 *      Author: utnso
 */

#ifndef SERVER_H_
#define SERVER_H_
#include "sockets-utils.h"

typedef struct server
{
	int socket_asociado;
	int backlog;
} t_server;

t_server* server_create(int puerto, char *ip, int backlog);
int server_asociate_a_puerto(int server, address_config_in *address);
void server_escucha(t_server *server);
void server_cerra_cliente(int cliente);
int server_acepta_conexion_cliente(t_server *server);

#endif /* SERVER_H_ */
