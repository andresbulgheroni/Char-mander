/*
 * lectura_escritura.c
 *
 *  Created on: 12/10/2016
 *      Author: utnso
 */
#include "lectura_escritura.h"
extern int *table_asignaciones;

/*----------------------------------------------TRUNCATE----------------------------------------------------------*/
void osada_b_truncate_file_full(t_to_be_truncate *to_truncate,osada_file *file,t_info_file *info)
{
	if(to_truncate->new_size >= file->file_size)
	{
		osada_b_aumentar_tamanio_full(file,to_truncate->new_size,info);
	}
	else
	{
		osada_b_disminuir_tamanio_full(file,to_truncate->new_size,info);
	}
}

void osada_b_aumentar_tamanio_full(osada_file *file, int new_size,t_info_file *info)
{
	int tamanio_a_aumentar;
	if(new_size == file->file_size)
	{
		tamanio_a_aumentar=new_size;
	}
	else
	{
			tamanio_a_aumentar = new_size - file->file_size;
	}

	int bloques_necesarios = tamanio_a_aumentar / OSADA_BLOCK_SIZE;

	if(bloques_necesarios == 0)
	{
			asignar_un_unico_bloque_si_es_necesario_full(file,info,tamanio_a_aumentar);
	}
	else
	{
			if(file->first_block== FEOF)
			{
				asignar_un_unico_bloque_si_es_necesario_full(file,info,OSADA_BLOCK_SIZE);
				if(tamanio_a_aumentar > (OSADA_BLOCK_SIZE*bloques_necesarios))
				{
					asignar_nuevo_bloque_datos_full(file,info,bloques_necesarios);
				}
				else
				{
					asignar_nuevo_bloque_datos_full(file,info,bloques_necesarios-1);
				}
			}
			else
			{

				if(tamanio_a_aumentar > (OSADA_BLOCK_SIZE*bloques_necesarios))
				{
					int espacio_disponible = calcular_espacio_disponible_ultimo_bloque(file);
					if(espacio_disponible != 0)
					{
						int a_escribir_en_bloque_incompleto = espacio_disponible - (tamanio_a_aumentar -(OSADA_BLOCK_SIZE*bloques_necesarios));
						if(a_escribir_en_bloque_incompleto <= espacio_disponible && a_escribir_en_bloque_incompleto>0)
						{
							asignar_nuevo_bloque_datos_full(file,info,bloques_necesarios);
						}
						else {asignar_nuevo_bloque_datos_full(file,info,bloques_necesarios +1);}
					}
					else
					{
						asignar_nuevo_bloque_datos_full(file,info,bloques_necesarios+1);
					}

				}
				else { asignar_nuevo_bloque_datos_full(file,info,bloques_necesarios);}
			}
		}
}

void asignar_un_unico_bloque_si_es_necesario_full(osada_file *file, t_info_file *info,int tamanio_a_aumentar)
{
	if(file->first_block == FEOF)
	{
			osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);

			int num_bloque_absoluto = osada_ocupa_bit_libre_de(disco);
			int bloque_Asignado = calcular_bloque_relativo_datos_dado_absoluto(num_bloque_absoluto);

			settear_valor_en_tabla_asginaciones(bloque_Asignado, FEOF);

			limpiar_bloque_de_datos(bloque_Asignado);
			int offset_del_recien_asignado = byte_inicial_tabla_asignaciones + 4*bloque_Asignado;
			int *feof = malloc(4);
			*feof = FEOF;

			memcpy(disco->map + offset_del_recien_asignado,feof,4);

			free(feof);

			file->first_block = bloque_Asignado;
			file->file_size=OSADA_BLOCK_SIZE;
			info->last_block_asigned = bloque_Asignado;
			info->cantidad_bloques_asignados = info->cantidad_bloques_asignados +1;
			info->last_block_write = FEOF;
			osada_impactar_un_archivo(info->posicion_en_tabla_de_archivos,file);
		}
		else
		{
			int espacio_disponible = calcular_espacio_disponible_ultimo_bloque(file);
			int capacidad_satisfaccion = espacio_disponible - tamanio_a_aumentar;
			if(capacidad_satisfaccion<0)
			{
				asignar_nuevo_bloque_datos_full(file,info,1);
			}
		}
}

void asignar_nuevo_bloque_datos_full(osada_file * file, t_info_file *info,int n)
{
	int *ultimo_elemento = malloc(sizeof(int));
	*ultimo_elemento = info->last_block_asigned;

	int i=0;
	while(i<n)
	{
		int num_bloque_absoluto = osada_ocupa_bit_libre_de(disco);
		int bloque_Asignado = calcular_bloque_relativo_datos_dado_absoluto(num_bloque_absoluto);
		limpiar_bloque_de_datos(bloque_Asignado);
		impactar_en_tabla_de_asignaciones(*ultimo_elemento,bloque_Asignado);
		*ultimo_elemento = bloque_Asignado;
		i++;
	}
	file->file_size = (file->file_size+ (n*OSADA_BLOCK_SIZE));
	info->last_block_asigned = *ultimo_elemento;
	info->cantidad_bloques_asignados = info->cantidad_bloques_asignados + n;
	osada_impactar_un_archivo(info->posicion_en_tabla_de_archivos,file);
	free(ultimo_elemento);
}

void impactar_en_tabla_de_asignaciones(int posicion, int valor)
{
	settear_valor_en_tabla_asginaciones(posicion, valor);
	settear_valor_en_tabla_asginaciones(valor, FEOF);
	osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);
	int desplazamiento = byte_inicial_tabla_asignaciones + 4*posicion;
	int *value = malloc(4);
	*value = valor;
	memcpy(disco->map + desplazamiento,value,4);
	free(value);

	int offset_del_recien_asignado = byte_inicial_tabla_asignaciones + 4*valor;
	int *feof = malloc(4);
	*feof = FEOF;
	memcpy(disco->map + offset_del_recien_asignado,feof,4);
	free(feof);
}

int osada_check_space_to_truncate_full(osada_file *file,t_info_file *info,int size)
{
	if(size< file->file_size)
	{
			return 1;
	}
	else
	{
			int blOques_necesarios_para_satisfacer = size / OSADA_BLOCK_SIZE;
			if(blOques_necesarios_para_satisfacer==0)
			{
				blOques_necesarios_para_satisfacer = 1;
			}

			if(file->file_size==0)
			{
				if(size > blOques_necesarios_para_satisfacer* OSADA_BLOCK_SIZE)
				{
						return osada_b_check_is_bitarray_have_n_blocks_free(blOques_necesarios_para_satisfacer+1,disco);
				}
				else
				{
					return osada_b_check_is_bitarray_have_n_blocks_free(blOques_necesarios_para_satisfacer,disco);
				}
			}
			else
			{
				int cantidad_bloques_actuales = info->cantidad_bloques_asignados;
				if(size >= (blOques_necesarios_para_satisfacer - cantidad_bloques_actuales)* OSADA_BLOCK_SIZE)
				{
					if(blOques_necesarios_para_satisfacer==cantidad_bloques_actuales)
					{
					return osada_b_check_is_bitarray_have_n_blocks_free(blOques_necesarios_para_satisfacer,disco);
					}
					else
					{
						return osada_b_check_is_bitarray_have_n_blocks_free((blOques_necesarios_para_satisfacer-cantidad_bloques_actuales),disco);
					}

				}
				else
				{
					return osada_b_check_is_bitarray_have_n_blocks_free((blOques_necesarios_para_satisfacer-cantidad_bloques_actuales),disco);
				}
			}
	}
}

void osada_b_disminuir_tamanio_full(osada_file *file, int new_size,t_info_file *info)
{
	int bloques_a_liberar = cantidad_de_bloques_a_desasignar(file->file_size,new_size);
	liberar_n_bloques_full(file,bloques_a_liberar,info);
	osada_b_change_size_file_full(file,new_size,info);
	info->cantidad_bloques_asignados = (info->cantidad_bloques_asignados) - bloques_a_liberar;
}

void liberar_n_bloques_full(osada_file *file, int bloques_a_liberar,t_info_file *info)
{
	t_list *bloques_actuales = osada_get_blocks_nums_of_this_file(file,disco);
	int cantidad_bloques_actuales = list_size(bloques_actuales);

	int i = cantidad_bloques_actuales -1;
	int stop = cantidad_bloques_actuales - bloques_a_liberar;
	if(stop!=0)
	{
		int cantidad_bloques_admin = calcular_cantidad_bloques_admin();
		while(i>=stop)
		{
			int *num_block = list_get(bloques_actuales,i);
			bitarray_clean_bit(disco->bitmap,cantidad_bloques_admin + (*num_block));
			i--;
		}
		osada_aumenta_cantidad_bloques_libres(bloques_a_liberar);
		impactar_en_disco_n_bloques(OSADA_BLOCK_SIZE,disco->header->bitmap_blocks,disco->bitmap->bitarray,disco->map);

		int *last_new_block = list_get(bloques_actuales,i);
		establecer_nuevo_feof_en_tabla_de_asignaciones(*last_new_block);
		settear_valor_en_tabla_asginaciones(*last_new_block, FEOF); //AGREGADO!
		info->last_block_asigned = *last_new_block;

	}
	else
	{
		osada_desocupa_n_bits(bloques_actuales);
		file->first_block = FEOF;
		osada_impactar_un_archivo(info->posicion_en_tabla_de_archivos,file);
		info->last_block_asigned = FEOF;
		info->last_block_write = FEOF;
	}
	list_destroy_and_destroy_elements(bloques_actuales,free_list_blocks);
}

int cantidad_de_bloques_a_desasignar(uint32_t tamanio_actual, int new_size)
{
		int bloques_actuales = tamanio_actual / OSADA_BLOCK_SIZE;
		if(tamanio_actual>bloques_actuales*OSADA_BLOCK_SIZE)
		{
			bloques_actuales =bloques_actuales + 1;
		}
		else
		{
			if(tamanio_actual<OSADA_BLOCK_SIZE)
			{
				bloques_actuales=1;
			}
		}

		int nuevos_bloques_requeridos;
		if(new_size<OSADA_BLOCK_SIZE)
		{
			if(new_size==0)
			{
				nuevos_bloques_requeridos=0;
			}
			else
			{
				nuevos_bloques_requeridos = 1;
			}
		}
		else
		{
			int cantidad_entera_requerida = new_size / OSADA_BLOCK_SIZE;
			if(new_size>cantidad_entera_requerida*OSADA_BLOCK_SIZE)
			{
				nuevos_bloques_requeridos = cantidad_entera_requerida +1;
			}
			else
			{
				nuevos_bloques_requeridos = cantidad_entera_requerida;
			}
		}

	return (bloques_actuales - nuevos_bloques_requeridos);
}
void establecer_nuevo_feof_en_tabla_de_asignaciones(int posicion)
{
	osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);
	int desplazamiento = byte_inicial_tabla_asignaciones + 4*posicion;
	int *value = malloc(4);
	*value = FEOF;
	memcpy(disco->map + desplazamiento,value,4);
	free(value);
}

int calcular_bloque_relativo_datos_dado_absoluto(int numero_bloque_absoluto)
{
	int bloque_incial = osada_get_start_block_absolut_of(BLOQUE_DE_DATOS,disco);
	int bloque_relativo = numero_bloque_absoluto - bloque_incial;
	return bloque_relativo;
}

/*----------------------------------------------ESCRITURA----------------------------------------------------------*/
int calcular_espacio_disponible_ultimo_bloque(osada_file *file)
{
	int bloques_enteros = file->file_size / OSADA_BLOCK_SIZE;
	int espacio_disponible_ultimo_bloque;
	if(bloques_enteros == 0)
	{
		espacio_disponible_ultimo_bloque = OSADA_BLOCK_SIZE - file->file_size;
		return espacio_disponible_ultimo_bloque;
	}
	else
	{
		if(file->file_size  == (OSADA_BLOCK_SIZE*bloques_enteros))
		{
			return 0;
		}
		else
		{
			espacio_disponible_ultimo_bloque = OSADA_BLOCK_SIZE - (file->file_size - (bloques_enteros * OSADA_BLOCK_SIZE));
			return espacio_disponible_ultimo_bloque;
		}
	}
}

int es_multiplo_de(int numero_1, int numero_2)
{
	int division = numero_1 / numero_2;
	if(numero_1 == (division*numero_2))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void osada_write_little_file_full_(t_to_be_write* file, osada_file *archivo, t_info_file *info)
{
	int tamanio_del_archivo = file->size_inmediatamente_anterior;
	t_list *bloques = osada_get_blocks_nums_of_this_file(archivo,disco);
	int cantidad_bloques = list_size(bloques);
	void *data = osada_get_data_of_this_file(archivo,disco,bloques);
	void *aux = malloc(OSADA_BLOCK_SIZE * cantidad_bloques);
	if(file->offset == 0)
	{
		memcpy(aux, file->text, file->size);
		osada_b_alter_data_blocks_full(aux,bloques);
		osada_b_change_size_file_full(archivo, file->size,info);
	}
	else
	{
		if(file->offset < tamanio_del_archivo)
		{
				memcpy(aux,data,file->offset);
				memcpy(aux + file->offset, file->text, file->size);
				memcpy(aux + (file->offset + file->size),data + file->offset, (tamanio_del_archivo - file->offset));
				osada_b_alter_data_blocks_full(aux,bloques);
				osada_b_change_size_file_full(archivo, file->size + tamanio_del_archivo,info);
		}
		else
		{
				memcpy(aux,data,archivo->file_size);
				memcpy(aux + file->offset, file->text, file->size);
				osada_b_alter_data_blocks_full(aux,bloques);
				osada_b_change_size_file_full(archivo, file->size + tamanio_del_archivo,info);
		}
	}

		free(aux);
		free(data);
		int* ultimo_bloque = list_get(bloques,cantidad_bloques -1);
		info->last_block_write = *ultimo_bloque;
		list_destroy_and_destroy_elements(bloques,free_list_blocks);
}

void osada_b_alter_data_blocks_full(void *data_new, t_list *bloques)
{
	int cantidad_bloques = list_size(bloques);

	int i = 0;
	while(i<cantidad_bloques)
	{
		int *num_block = list_get(bloques,i);
		void *data_aux = malloc(OSADA_BLOCK_SIZE);
		memcpy(data_aux, data_new + (i*OSADA_BLOCK_SIZE),OSADA_BLOCK_SIZE);
		osada_push_block(BLOQUE_DE_DATOS,*num_block,data_aux,disco);
		free(data_aux);
		i++;
	}
}

void osada_b_change_size_file_full(osada_file *file,int new_size, t_info_file *info)
{
	uint32_t size = (uint32_t) new_size;
	file->file_size = size;
	osada_impactar_un_archivo(info->posicion_en_tabla_de_archivos,file);
}

void osada_write_big_file_full_(t_to_be_write* to_write, osada_file *archivo, t_info_file *info)
{
	t_list *bloques = osada_get_blocks_nums_of_this_file_since_full(info->last_block_write);;
	int cantidad_bloques = list_size(bloques);

	int index = 1;
	int i=0;
	while(index<cantidad_bloques)
	{
		int *num_block = list_get(bloques,index);
		void *data_aux = malloc(OSADA_BLOCK_SIZE);
		memcpy(data_aux,to_write->text + (i*OSADA_BLOCK_SIZE),OSADA_BLOCK_SIZE);
		osada_push_block(BLOQUE_DE_DATOS,*num_block,data_aux,disco);
		free(data_aux);
		i++;
		index++;
	}
	osada_b_change_size_file_full(archivo,to_write->size_inmediatamente_anterior + to_write->size,info);
	info->last_block_write = info->last_block_asigned;
	list_destroy_and_destroy_elements(bloques, free_list_blocks);
}


int ultimo_bloque_escrito(t_file_osada* file)
{
	int ultimo_bloque_escrito = file->file->file_size / OSADA_BLOCK_SIZE;
	if(file->file->file_size > (ultimo_bloque_escrito*OSADA_BLOCK_SIZE) )
	{
		ultimo_bloque_escrito = ultimo_bloque_escrito+1;
	}
	return ultimo_bloque_escrito;
}

void osada_b_alter_data_blocks(t_file_osada *file, void *data_new, t_list *bloques)
{
	int cantidad_bloques = list_size(bloques);

	int i = 0;
	while(i<cantidad_bloques)
	{
		int *num_block = list_get(bloques,i);
		void *data_aux = malloc(OSADA_BLOCK_SIZE);
		memcpy(data_aux, data_new + (i*OSADA_BLOCK_SIZE),OSADA_BLOCK_SIZE);
		osada_push_block(BLOQUE_DE_DATOS,*num_block,data_aux,disco);
		free(data_aux);
		i++;
	}
}

/*---------------------------------------------LECTURA------------------------------------------------------------*/
void* osada_b_read_file(osada_file *file, t_disco_osada *disco, t_to_be_read *to_read, t_list* bloques_por_recuperar )
{
	if(file->first_block==FEOF)
	{
		return NO_EXISTE;
	}
	else
	{
		int bloque_inicial =  (to_read->offset/OSADA_BLOCK_SIZE);
		int desplazamiento_en_bloque_init = to_read->offset - (bloque_inicial * OSADA_BLOCK_SIZE);

		int bloque_final = calcular_bloque_final_por_leer(bloque_inicial,to_read);

		void *data = malloc(to_read->size);
		if(bloque_inicial == bloque_final)
		{
			int bloque_num = buscar_bloque_numero(file,bloque_inicial);
			void *data_recv = osada_get_blocks_relative_since(BLOQUE_DE_DATOS,bloque_num,1,disco);
			memcpy(data,data_recv+to_read->offset,to_read->size);
			free(data_recv);
			return( (void*) data);
		}
		else
		{
			int i = bloque_inicial;
			int iteracion = 0;

			if(to_read->size <= OSADA_BLOCK_SIZE*(bloque_final-bloque_inicial))
			{
						bloque_final = bloque_final -1;
			}
			int block_num = buscar_bloque_numero(file,bloque_inicial);
			while(i<=bloque_final)
			{
				void *data_recv = osada_get_blocks_relative_since(BLOQUE_DE_DATOS,block_num,1,disco);
				if(i == bloque_final)
				{
					int cantidad_bloques_enteros_leidos = to_read->size / OSADA_BLOCK_SIZE;
					int bytes_a_leer_del_ultimo_bloque = to_read->size - (cantidad_bloques_enteros_leidos*OSADA_BLOCK_SIZE);
					if(bytes_a_leer_del_ultimo_bloque==0)
					{
						memcpy(data + (iteracion*OSADA_BLOCK_SIZE),data_recv,OSADA_BLOCK_SIZE);
					}
					else
					{
						memcpy(data + (iteracion*OSADA_BLOCK_SIZE),data_recv,bytes_a_leer_del_ultimo_bloque);
					}

				}
				else
				{
					if(i == bloque_inicial)
					{
						memcpy(data + (desplazamiento_en_bloque_init),data_recv,OSADA_BLOCK_SIZE);
					}
					else
					{
						memcpy(data + (iteracion*OSADA_BLOCK_SIZE),data_recv,OSADA_BLOCK_SIZE);
					}
				}
					block_num = table_asignaciones[block_num];
					free(data_recv);
					iteracion++;
					i++;
				}
				return data;
			}
	}
}


int calcular_bloque_final_por_leer(int bloque_inicial,t_to_be_read *to_read)
{

	int bloque_final;
	if(to_read->size<= OSADA_BLOCK_SIZE -1)
	{
		bloque_final = bloque_inicial;
		return bloque_final;
	}
	else
	{
		int cantidad_bloques_enteros_leidos = to_read->size / OSADA_BLOCK_SIZE;
		if(to_read->size> cantidad_bloques_enteros_leidos*OSADA_BLOCK_SIZE)
		{
			return (bloque_inicial +cantidad_bloques_enteros_leidos +1 );
		}
		else
		{
			return (bloque_inicial +cantidad_bloques_enteros_leidos );
		}
	}
}


void* osada_get_data_of_this_file(osada_file *file, t_disco_osada *disco, t_list *bloques_por_recuperar)
{
	//t_list *bloques_por_recuperar = osada_get_blocks_nums_of_this_file(file, disco);
	int size = list_size(bloques_por_recuperar);

	int last_block = size -1;
	void *data = malloc(size*OSADA_BLOCK_SIZE);
	int i;
	for(i=0; i<(size); i++)
	{
		int *block_num = list_get(bloques_por_recuperar,i);
		void *data_recv = osada_get_blocks_relative_since(BLOQUE_DE_DATOS,*block_num,1,disco);

		if(i == last_block)
		{
			int ultimo_byte_a_recibir = calcular_byte_final_a_recuperar_de_file(file->file_size);
			if(ultimo_byte_a_recibir == OSADA_BLOCK_SIZE)
			{
				memcpy(data + (i*OSADA_BLOCK_SIZE),data_recv,ultimo_byte_a_recibir);
				free(data_recv);
			}
			else
			{
				memcpy(data + (i*OSADA_BLOCK_SIZE),data_recv,ultimo_byte_a_recibir);
				free(data_recv);
			}
		}
		else
		{
			memcpy(data + (i*OSADA_BLOCK_SIZE),data_recv,OSADA_BLOCK_SIZE);
			free(data_recv);

		}
	}
	//list_destroy_and_destroy_elements(bloques_por_recuperar,free_list_blocks);
	return( (void*) data);
}


int calcular_byte_final_a_recuperar_de_file(int file_size)
{
	if(file_size < OSADA_BLOCK_SIZE)
	{
		return file_size;
	}
	else
	{
		int bloques_completos =  file_size / OSADA_BLOCK_SIZE;
		if(bloques_completos * OSADA_BLOCK_SIZE == file_size)
		{
			return (OSADA_BLOCK_SIZE);
		}
		else
		{
			int byte_final = file_size -  OSADA_BLOCK_SIZE * bloques_completos;
			return byte_final;
		}

	}
}
