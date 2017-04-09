/*
 * vars.h
 *
 *  Created on: 23/10/2016
 *      Author: utnso
 */

#ifndef VARS_H_
#define VARS_H_

#include <errno.h>

typedef struct
{
	int puerto;
	char* ip;
	int socket_pokedex_servidor;
}t_cliente_osada;

t_cliente_osada* cliente_osada;

enum
{
	ARCHIVO =1,
	DIRECTORIO =2
};

enum
{
	EXITO = 1,
	NO_EXISTEE = 2,
	EXISTEE = 3,
	NO_HAY_ESPACIOO = 4,
	ARGUMENTO_INVALIDOO = 5
};

/*
 * REDEFINIMOS FLAGS DE RETORNO
 */
enum
{
	NO_EXISTE =ENOENT,
	EXISTE =EEXIST,
	 NO_HAY_ESPACIO =ENOSPC,
	ARGUMENTO_INVALIDO =EINVAL,
	 OPERACION_EXITOSA =0,
	NO_SE_PUEDEN_CREAR_MAS_ARCHIVOS = EDQUOT,
	ARCHIVO_MUY_GRANDE = EFBIG
};


#endif /* VARS_H_ */
