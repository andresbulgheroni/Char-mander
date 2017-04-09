/*
 * operaciones.c
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */
#include "operaciones.h"

extern pthread_mutex_t mutex_por_archivo[];
extern pthread_mutex_t mutex_por_archivo_borrado[];

/*-------------------------------------------ATRIBUTOS------------------------------------------------------------------*/
void* osada_a_get_attributes(char *path)
{
	if(string_equals_ignore_case(path,"/"))
	{
		t_attributes_file *atributos = malloc(sizeof(t_attributes_file));
		atributos->tipo=2;
		t_info_file *info_raiz = dictionary_get(disco->diccionario_de_archivos,"/");
		atributos->size = info_raiz->tamanio_del_directorio;
		return atributos;
	}
	else
	{
		if(osada_check_exist(path))
		{
			t_attributes_file *atributos = osada_b_get_attributes_of_this_file(path);
			return atributos;
		}
		else
		{

			return NO_EXISTE;
		}
	}
}

void* osada_a_get_list_dir(char *path)
{
	if(string_equals_ignore_case(path,"/"))
	{
		t_list *listado = osada_b_listar_hijos(path);
		return listado;
	}
	else
	{
		if(osada_check_exist(path))
			{
				t_list *listado = osada_b_listar_hijos(path);
				return listado;
			}
		else
		{
			return NO_EXISTE;
		}
	}

}

/*-------------------------------------------CREACION-------------------------------------------------------------------*/
void* osada_a_create_file(char *path)
{
	if(osada_b_check_name(path))
	{
		if(osada_b_check_repeat_name(REGULAR,path))
		{
			int hay_lugar = osada_hay_espacio_para_archivos();

			if(hay_lugar)
			{
				t_osada_file_free *new_file=osada_b_file_create(REGULAR,path);
				int offset = calcular_desplazamiento_tabla_de_archivos(new_file->position_in_block);
				osada_push_middle_block(TABLA_DE_ARCHIVOS,new_file->block_relative,offset,new_file->file,disco);
				osada_disminui_cantidad_de_archivos();
				t_file_osada_destroy((t_file_osada*) new_file);

				return EXITO;
			}
			else
			{
				return NO_HAY_ESPACIO;
			}
		}
		else
		{
			return EXISTE;
		}
	}
	else
	{
		return ARGUMENTO_INVALIDO;
	}
}

void* osada_a_create_dir(char *path)
{
	if(osada_b_check_name(path))
	{
		if(osada_b_check_repeat_name(DIRECTORY,path))
		{
			int hay_lugar = osada_hay_espacio_para_archivos();
			if(hay_lugar)
			{
				t_osada_file_free *new_file=osada_b_file_create(DIRECTORY,path);
				int offset = calcular_desplazamiento_tabla_de_archivos(new_file->position_in_block);
				osada_push_middle_block(TABLA_DE_ARCHIVOS,new_file->block_relative,offset,new_file->file,disco);
				osada_disminui_cantidad_de_archivos();
				t_file_osada_destroy((t_file_osada*) new_file);
				return EXITO;
			}
			else
			{
				return NO_HAY_ESPACIO;
			}
		}
		else
		{
			return EXISTE;
		}
	}
	else
	{
		return ARGUMENTO_INVALIDO;
	}
}

/*-------------------------------------------ELMINACION-----------------------------------------------------------------*/
void* osada_a_delete_file(char *path)
{
	if(osada_check_exist(path))
	{
		osada_delete_this_file(path);
		return EXITO;
	}
	else
	{
		return NO_EXISTE;
	}
}

void* osada_a_delete_dir(char *path)
{
	if(osada_check_exist(path))
	{
		osada_delete_this_dir(path);
		return EXITO;
	}
	else
	{
		return NO_EXISTE;
	}
}

/*-------------------------------------------WRITE & READ---------------------------------------------------------------*/
void* osada_a_read_file(t_to_be_read *to_read)
{
	if(osada_check_exist(to_read->path))
	{
		t_info_file *info = dictionary_get(disco->diccionario_de_archivos,to_read->path);
		osada_file *file = osada_get_file_for_index(info->posicion_en_tabla_de_archivos);

		int size = file->file_size;
		if(size == 0 || to_read->offset == size)
		{
			free(file);
			return ARGUMENTO_INVALIDO;
		}
		else
		{
			read_content *read = malloc(sizeof(read_content));
			int tamanio_final = to_read->offset + to_read->size;
			if(size<tamanio_final)
			{
				if(to_read->offset==0)
				{
					int tamanio_que_pidio_leer = to_read->size;
					to_read->size = size; //CAMBIO EL TAMAÑO QUE VA A LEER
					void* data = osada_b_read_file(file,disco,to_read,NULL);
					read->contenido = data;
					read->tamanio = to_read->size;
					free(file);
					return read;
				}
				else
				{
					int tamanio_que_pidio_leer = to_read->size;
					to_read->size = size - to_read->offset;
					void* data = osada_b_read_file(file,disco,to_read,NULL);
					read->contenido = data;
					read->tamanio = to_read->size;
					free(file);
					return read;
				}
			}
			else
			{
				void* data = osada_b_read_file(file,disco,to_read,NULL);
				read->contenido=data;
				read->tamanio=to_read->size;
				free(file);
				return read;
			}
		}
	}
	else
	{
		return NO_EXISTE;
	}
}

void* osada_a_write_file(t_to_be_write *to_write)
{
	if(osada_check_exist(to_write->path))
	{
		t_info_file *info_file = dictionary_get(disco->diccionario_de_archivos,to_write->path);
		osada_file *file = osada_get_file_for_index(info_file->posicion_en_tabla_de_archivos);
		pthread_mutex_lock(&mutex_por_archivo_borrado[info_file->posicion_en_tabla_de_archivos]);
		int new_size_to_truncate = to_write->size + file->file_size;
		if(osada_check_space_to_truncate_full(file,info_file,new_size_to_truncate))
		{
			actualizar_tamanio_del_padre(info_file,to_write->size);

			t_to_be_truncate *truncate = malloc(sizeof(t_to_be_truncate));
			to_write->size_inmediatamente_anterior = file->file_size;
			if(to_write->offset == 0 && file->file_size>=0)
			{
				truncate->new_size = to_write->size;
			}
			else
			{
				truncate->new_size = to_write->size + file->file_size;
			}
			osada_b_truncate_file_full(truncate,file,info_file);
			if(to_write->size_inmediatamente_anterior !=0)
			{
				//if(to_write->size == PAGE_SIZE_MAX && es_multiplo_de(to_write->offset,to_write->size_inmediatamente_anterior) )
				if((es_multiplo_de(to_write->size,PAGE_SIZE_MAX) || es_multiplo_de(to_write->size,PAGE_SIZE_MAX/4)) && es_multiplo_de(to_write->offset,to_write->size_inmediatamente_anterior) )
				{
					osada_write_big_file_full_(to_write,file,info_file);
				}
				else
				{
					osada_write_little_file_full_(to_write,file,info_file);
				}
			}
			else
			{
				osada_write_little_file_full_(to_write,file,info_file);
			}
			//osada_b_actualiza_time(to_write->file);
			pthread_mutex_unlock(&mutex_por_archivo_borrado[info_file->posicion_en_tabla_de_archivos]);
			free(file);
			free(truncate);
			return EXITO;
		}
		else
		{
			pthread_mutex_unlock(&mutex_por_archivo_borrado[info_file->posicion_en_tabla_de_archivos]);
			free(file);
			return NO_HAY_ESPACIO;
		}
	}
	else
	{
		return NO_EXISTE;
	}

}

/*-------------------------------------------RENAME---------------------------------------------------------------------*/
void* osada_a_rename(t_to_be_rename *to_rename)
{

	if(osada_check_exist(to_rename->old_path))
	{
		if(osada_b_check_name(to_rename->new_path))
		{
			if(osada_b_check_repeat_name(NULL, to_rename->new_path))
				{

					osada_b_rename_full(to_rename);

					return EXITO;
				}
			else
				{

					return ARGUMENTO_INVALIDO;
				}
		}
		else
			{

				return ARGUMENTO_INVALIDO;
			}
	}
	else
	{
		return NO_EXISTE;
	}
}

/*-------------------------------------------OPENS & CLOSER-------------------------------------------------------------*/
void* osada_a_open_file(char *path)
{
	if(string_equals_ignore_case(path,"/"))
	{
		return EXITO;
	}
	else
	{

		if(osada_check_exist(path))
		{

			return EXITO;
		}
		else
		{
				return NO_EXISTE;
		}
	}

}

/*-------------------------------------------TRUNCATE-------------------------------------------------------------------*/
void* osada_a_truncate_file(char* path, int new_size)
{
	if(osada_check_exist(path))
	{
		t_info_file *info_file = dictionary_get(disco->diccionario_de_archivos,path);
		osada_file *file = osada_get_file_for_index(info_file->posicion_en_tabla_de_archivos);
		pthread_mutex_lock(&mutex_por_archivo[info_file->posicion_en_tabla_de_archivos]);
		if(new_size == 0 && file->file_size == 0)
		{
			pthread_mutex_unlock(&mutex_por_archivo[info_file->posicion_en_tabla_de_archivos]);
			free(file);
			return EXITO;
		}
		else
		{
			if(osada_check_space_to_truncate_full(file,info_file,new_size))
			{
				actualizar_tamanio_del_padre(info_file,-(file->file_size));
				actualizar_tamanio_del_padre(info_file,new_size);

				t_to_be_truncate* trunct = malloc(sizeof(t_to_be_truncate));
				trunct->path=path;
				trunct->new_size = new_size;
				osada_b_truncate_file_full(trunct,file,info_file);
				file->file_size = new_size;
				osada_impactar_un_archivo(info_file->posicion_en_tabla_de_archivos,file);
				//osada_b_actualiza_time(file);
				pthread_mutex_unlock(&mutex_por_archivo[info_file->posicion_en_tabla_de_archivos]);
				free(trunct);
				free(file);
				return EXITO;
			}
			else
			{
				pthread_mutex_unlock(&mutex_por_archivo[info_file->posicion_en_tabla_de_archivos]);
				free(file);
				return NO_HAY_ESPACIO;
			}
		}
		}
	else
	{
		return NO_EXISTE;
	}

}




