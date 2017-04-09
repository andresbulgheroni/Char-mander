/*
 * serializacion_mapa_entrenador.h
 *
 *  Created on: 25/9/2016
 *      Author: utnso
 */

#ifndef SERIALIZACION_MAPA_ENTRENADOR_H_
#define SERIALIZACION_MAPA_ENTRENADOR_H_
#include "../so-commons/string.h"
#include "sockets-utils.h"

char* recibir_mensaje_especifico(int socket);
char* armar_mensaje(char *header, char *payload, int max_bytes);
char* armar_coordenada(int x, int y, int max_bytes);

/*-------------------------------------------MORE STRINGS FUNCTIONS------------------------------------------------------*/
void free_string_array(char **path);
int string_contains(char *palabra, char *conteiner);
char* string_replace(char **palabra, char *este_caracter,char *por_este);
char* string_path_replace_spaces(char **path, char *este_caracter, char *por_este);
#endif /* SERIALIZACION_MAPA_ENTRENADOR_H_ */
