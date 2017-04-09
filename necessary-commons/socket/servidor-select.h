/*
 * servidor-select.h
 *
 *  Created on: 14/11/2016
 *      Author: utnso
 */

#ifndef SOCKET_SERVIDOR_SELECT_H_
#define SOCKET_SERVIDOR_SELECT_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include "string.h"
#include "sockets-utils.h"
#define YES 1
#define NO 0



typedef struct sockaddr_in address_config_in;


//ESTRUCTURA PARA UN SERVIDOR QUE SE IMPLEMENTARÁ CON SELECT//
typedef struct
{
	int socket_asociado;
	int backlog;
	fd_set *readset;
	int numero_clientes_conectados;
	int *sockets_clientes_conectados;
	int maximo_descriptor;
} t_socket_server;

typedef struct sockaddr_in address_config_in;

						//FUNCIONES PRINCIPALES//
void ejecutar_server_select(int puerto, int backlog, int longpaquete, void (*funcion_de_atencion)(int parametro));
address_config_in* configurar_address_in_select(int puerto);

t_socket_server* server_stream_create(int puerto, int backlog);
int server_stream_asociate_a_puerto(int server, address_config_in *address);
void server_stream_escucha(t_socket_server *server);
void server_stream_destruite(t_socket_server *server);
void server_stream_nuevo_cliente (t_socket_server *server,void (*funcion_de_atencion)(int parametro) );
void server_stream_atende_clientes(t_socket_server *server,fd_set *descriptorTemporal,int longitudPaquete, void (*funcion_de_atencion)(int parametro));

						//FUNCIONES SECUNDARIAS//

/*MANIPULACION DE READSET */
fd_set* readset_create();
void server_agrega_cliente_a_readset(t_socket_server *server, int cliente);
void server_quita_cliente_del_readset(t_socket_server *server, int cliente);
void server_actualiza_maximo_descriptor(t_socket_server *server);
void server_cambia_tu_maximo_descriptor(t_socket_server *server);

/* MANIPULACION DE CLIENTES CONECTADOS */
void server_aumenta_numero_de_conectados(t_socket_server *server, int cliente);
void server_modela_nuevo_cliente(t_socket_server *server);
int server_acepta_conexion_cliente_select(int socketServer);
void server_atende_cliente(t_socket_server *server, int cliente, int longitudPaquete, char *buffer);
void server_cerra_cliente_select(t_socket_server *server, int cliente);
int server_permitis_nuevo_cliente(int nClientesConectados, int backlog);

/*--------------------------------------------------AUXILIARES ARRAYS ------------------------------------------------------------*/
void inicializarArray(int *array, int tamanio);
void eliminar_elemento_del_array(int *array, int cantidadDeElementos, int elemento);
int mayor_del_array(int *array, int tamanio);
int arrayInt_maximo_entre(int *array, int tamanioArray, int numero);
void agregar_elemento_a_array(int *array, int posicion, int elemento);
int* arrayInt_create(int tamanio);

#endif /* SOCKET_SERVIDOR_SELECT_H_ */
