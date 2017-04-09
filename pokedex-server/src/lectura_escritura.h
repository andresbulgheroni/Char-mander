/*
 * lectura_escritura.h
 *
 *  Created on: 12/10/2016
 *      Author: utnso
 */

#ifndef LECTURA_ESCRITURA_H_
#define LECTURA_ESCRITURA_H_
#include "osada.h"
#include "file_manipuling.h"
#include "osada_generales.h"

enum
{
	HAY_ESPACIO_SUFICIENTE = 1,
	NO_HAY_ESPACIO_SUFICIENTE = 0
};

/*----------------------------------------------TRUNCATE----------------------------------------------------------*/
void osada_b_truncate_file_full(t_to_be_truncate *to_truncate,osada_file *file,t_info_file *info);
void osada_b_aumentar_tamanio_full(osada_file *file, int new_size,t_info_file *info);
void osada_b_disminuir_tamanio_full(osada_file *file, int new_size,t_info_file *info);
void asignar_un_unico_bloque_si_es_necesario_full(osada_file *file, t_info_file *info,int tamanio_a_aumentar);
void asignar_nuevo_bloque_datos_full(osada_file * file, t_info_file *info,int n);
int osada_check_space_to_truncate_full(osada_file *file,t_info_file *info,int size);
void impactar_en_tabla_de_asignaciones(int posicion, int valor);
void liberar_n_bloques_full(osada_file *file, int bloques_a_liberar,t_info_file *info);
void establecer_nuevo_feof_en_tabla_de_asignaciones(int posicion);
int calcular_bloque_relativo_datos_dado_absoluto(int numero_bloque_absoluto);

int cantidad_de_bloques_a_desasignar(uint32_t tamanio_actual, int new_size);
/*----------------------------------------------ESCRITURA----------------------------------------------------------*/
void osada_write_little_file_full_(t_to_be_write* file, osada_file *archivo, t_info_file *info);
void osada_write_big_file_full_(t_to_be_write* to_write, osada_file *archivo, t_info_file *info);
void osada_b_alter_data_blocks_full(void *data_new, t_list *bloques);
void osada_b_change_size_file_full(osada_file *file,int new_size, t_info_file *info);
void osada_b_alter_data_blocks(t_file_osada *file, void *data_new, t_list *bloques);
int calcular_espacio_disponible_ultimo_bloque(osada_file *file);
int ultimo_bloque_escrito(t_file_osada* file);
int es_multiplo_de(int numero_1, int numero_2);


/*----------------------------------------------LECTURA-----------------------------------------------------------*/
void* osada_get_data_of_this_file(osada_file *file, t_disco_osada *disco, t_list *bloques_por_recuperar);
int calcular_byte_final_a_recuperar_de_file(int file_size); //<-- La usa la función anteriror

void* osada_b_read_file(osada_file *file, t_disco_osada *disco, t_to_be_read *to_read, t_list* bloques_por_recuperar );
int calcular_bloque_final_por_leer(int bloque_inicial,t_to_be_read *to_read);
#endif /* LECTURA_ESCRITURA_H_ */
