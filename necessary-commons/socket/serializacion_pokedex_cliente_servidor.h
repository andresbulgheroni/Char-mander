/*
 * serializacion_pokedex_cliente_servidor.h
 *
 *  Created on: 6/10/2016
 *      Author: utnso
 */

#ifndef SOCKET_SERIALIZACION_POKEDEX_CLIENTE_SERVIDOR_H_
#define SOCKET_SERIALIZACION_POKEDEX_CLIENTE_SERVIDOR_H_
#include "../so-commons/string.h"
#include "sockets-utils.h"

#define MAX_BYTES_SIZE 1000000
#define MAX_BYTES_TO_SEND 10

enum
{
	LISTAR = 1,
	GET_ATRIBUTES =2,
	CREATE_DIRECTORY =3,
	CREATE_FILE=4,
	DELETE_FILE=5,
	DELETE_DIRECTTORY=6,
	READ_FILE=7,
	WRITE_FILE=8,
	RENAME_FILE=9,
	OPEN_FILE=10,
	TRUNCATE_FILEE=11
};

/*-----------------------------------------PETICIONES----------------------------------------------------*/
char* build_msg(int header, char *path_original, char *path_new_or_text, int size, int offset);
char* armar_numero_de_bytes(int size_payload);
char* armar_header(int header);
char* armar_lectura_o_escritura(int tipo,char *path, char *text, int size, int offset);

void* recibir_mensaje_escritura(int socket, int payloadsize);
/*-----------------------------------------SOBRE ERRORES-------------------------------------------------*/
#endif /* SOCKET_SERIALIZACION_POKEDEX_CLIENTE_SERVIDOR_H_ */
