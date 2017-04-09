/*
 * operaciones.h
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

#ifndef OPERACIONES_H_
#define OPERACIONES_H_
#include "osada.h"
#include "ls_and_atributes.h"
#include "osada_generales.h"
#include "lectura_escritura.h"
#include "comunication.h"

/*-------------------------------------------ATRIBUTOS------------------------------------------------------------------*/
void* osada_a_get_attributes(char *path);
void* osada_a_get_list_dir(char *path);
/*-------------------------------------------CREACION-------------------------------------------------------------------*/
void* osada_a_create_file(char *path);
void* osada_a_create_dir(char *path);
/*-------------------------------------------ELMINACION-----------------------------------------------------------------*/
void* osada_a_delete_file(char *path);
void* osada_a_delete_dir(char *path);
/*-------------------------------------------WRITE & READ---------------------------------------------------------------*/
void* osada_a_read_file(t_to_be_read *to_read);
void* osada_a_write_file(t_to_be_write *to_write);

/*-------------------------------------------RENAME---------------------------------------------------------------------*/
void* osada_a_rename(t_to_be_rename *to_rename);

/*-------------------------------------------OPENS & CLOSER-------------------------------------------------------------*/
void* osada_a_open_file(char *path);

/*-------------------------------------------TRUNCATE-------------------------------------------------------------------*/
void* osada_a_truncate_file(char* path, int new_size);
#endif /* OPERACIONES_H_ */
