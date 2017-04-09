/*
 * sockets-utils.h
 *
 *  Created on: 25/9/2016
 *      Author: utnso
 */

#ifndef SOCKETS_UTILS_H_
#define SOCKETS_UTILS_H_
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "string.h"
typedef struct sockaddr_in address_config_in;

address_config_in configurar_address_in(int puerto, char *ip);
void activar_reutilizacion_de_direcciones(int valorVerdaderoso, int socket);

int sendall(int s, char *buf, int *len);
void* reciveall(int sokcet, int len);

int seguir_enviado(int socket, void *buffer, int size);
void enviar_mensaje(int socket, char *mensaje);
void enviar_mensaje_cantidad_especifica(int socket, void *buffer, int size);
char* recibir_mensaje(int socket,int payloadSize);
void* recibir_mensaje_tipo_indistinto(int socket,int payloadsize);
char* recibir_mensaje_tipo_indistinto_string(int socket,int payloadsize);
void* recibir_mensaje_tipo_indistinto_2(int socket,int payloadsize);
void limpiar_buff_aux(char* buff,int tamanio);
#endif /* SOCKETS_UTILS_H_ */
