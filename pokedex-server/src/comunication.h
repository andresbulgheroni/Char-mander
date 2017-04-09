/*
 * comunication.h
 *
 *  Created on: 6/10/2016
 *      Author: utnso
 */

#ifndef COMUNICATION_H_
#define COMUNICATION_H_

#include "socket/serializacion_pokedex_cliente_servidor.h"
#include "socket/server.h"
#include "pthread.h"
#include "osada.h"
#include "operaciones.h"
#include "so-commons/string.h"
#include "socket/servidor-select.h"
#include "so-commons/log.h"

#define BYTES_TO_RCV 10
#define HEADER 2
#define MAX_CONECCTIONS 1500

/*---------------------------------CONEXION------------------------------------------*/
t_server* servidor_pokedex;

void ejecutar_servidor();
void pokedex_server_conectate();
//void pokedex_server_acepta_clientes();
void servidor_osada_crea_nuevo_cliente(int* cliente);

void servidor_acepta_clientes();
void* server_pokedex_atende_cliente(void* socket_cliente);

char* server_escucha_peticion(int cliente);
void tratar_peticion_de(int cliente,char *peticion);

/*--------------------------------------PETICIONES DEL CLIENTE-----------------------------------------------------*/
void* recibir_mensaje_especifico(int socket, int header);
char* escuchar_mensaje_operaciones_basicas(int socket);
t_to_be_read* escuchar_mensaje_read(int socket);
t_to_be_write* escuchar_mensaje_write(int socket);
t_to_be_rename* escuchar_mensaje_rename(int socket);
t_to_be_truncate* escuchar_mensaje_truncate(int socket);
/*--------------------------------------RESPUESTAS AL CLIENTE----------------------------------------------------*/
void responder_solo_resultado(int cliente, int resultado);
char* armar_attributes(t_attributes_file *attributes);
char* armar_listado(t_list *listado,char** num_bytes_so_send);
void modelar_cantidad_elementos_listado(char* buffer, int size);
char* modelar_elementos_en_listado(t_list *listado, int size, int* total_bytes_a_enviar);
char* modelar_tamanio_nombre(int size);
void agregar_barra_si_es_necesario(char *path);

#endif /* COMUNICATION_H_ */
