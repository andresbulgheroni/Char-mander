/*
 * borrados.h
 *
 *  Created on: 12/10/2016
 *      Author: utnso
 */

#ifndef BORRADOS_H_
#define BORRADOS_H_
#include "osada.h"
#include "osada_generales.h"
/*---------------------------------------------DELETE FILE--------------------------------------------------------------------*/
/*
 * @NAME:void osada_delete_this_file(char *path);
 * @DESC: Dado un PATH que lleva a un ARCHIVO, ELIMINA el mismo del disco.
 *
 * WARNING: AL LLAMAR A ESTA FUNCIÓN, SE PRESUME QUE EL PROGRAMADOR YA SE CHEQUEÓ LA EXISTENCIA DE DICHO PATH
 * 			(NO SE PUEDE BORRAR ALGO QUE NO ESTA EN DISCO). ADEMÁS, IMPACTA EN DISCO LOS CAMBIOS, POR LO QUE NO SE PODRÁ
 * 			VOLVER A RECUPERAR DICHO ARCHIVO.
 */
void osada_delete_this_file(char *path);
void delete_file(char *archivo); //<-- Simple delegación de la función anterior
int calcular_desplazamiento_tabla_de_archivos(int posicion_relativa); //<-- Simple delegación de la función anterior
int calcular_cantidad_bloques_admin(); //<-- Simple delegación
char* recuperar_path_padre(char* path);
void osada_recalculate_size_of(char* path, int tamanio_a_restar);

/*--------------------------------------DELETE DIR----------------------------------------------------------------------------*/

/*
 * @NAME: osada_delete_this_dir(char* path);
 * @DESC: Dado un path, chequea la existencia de ese directorio, borra a todos sus hijos, y se borra a si mismo,
 * 		  y cambia el tamaño del dir padre
 */
void osada_delete_this_dir(char* path);

/*
 * @NAME: osada_borrar_hijos(char* path);
 * @DESC: Dado un path, borra todos sus hijos, sean directorios o archivos.
 */
void osada_borrar_hijos(char* path);

/*
 * @NAME: osada_delete_dir_void(char* path);
 * @DESC: Dado un path de un directorio VACIO, setea en su tabla de archivos como borrado.
 */
void osada_delete_dir_void(char* path);

int es_directorio_vacio(char* path);


#endif /* BORRADOS_H_ */
