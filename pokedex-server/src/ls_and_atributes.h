/*
 * ls_and_atributes.h
 *
 *  Created on: 13/10/2016
 *      Author: utnso
 */

#ifndef LS_AND_ATRIBUTES_H_
#define LS_AND_ATRIBUTES_H_

#include "osada.h"
#include "osada_generales.h"
#include "borrados.h"
/*----------------------------------------------LISTAR--------------------------------------------------------------------*/
t_list* osada_b_listar_hijos(char* path);
int es_el_directorio_raiz(char *path);
void listar_directorio_raiz(t_list *lista);
void listar_directorio_comun(t_list *lista, char *path);
//void agregar_a_lista_si_es_hijo(t_file_osada *path_padre, osada_file* hijo, t_list* lista);
void agregar_a_lista_si_es_hijo_full(t_info_file *info_parent, int posicion_file_absoluto,t_list *lista);

void agregar_a_lista_si_es_hijo_de_raiz(int num_raiz, osada_file *file, t_list *lista);
void agregar_a_lista_si_es_hijo_de_raiz_full(int numero_bloque_absoluto,t_list *lista);


int verificar_si_son_mismo_files(osada_file *file_actual, osada_file *file_expected);
int es_el_padre(osada_file* file_hijo,t_file_osada *path_padre);

t_list* listar_hijos_para_renombrar(int numero_padre);
/*---------------------------------------------TAMAÑO DE UN DIRECTORIO---------------------------------------------------*/
int osada_b_calculate_size_of_directory(char *path_directory);
int realizar_sumatoria_size_hijos(char *path);
int tamanio_del_dir_raiz();
/*------------------------------------------ATRIBUTOS----------------------------------------------------------------------*/
t_attributes_file* osada_b_get_attributes_of_this_file(char *path_file);

#endif /* LS_AND_ATRIBUTES_H_ */
