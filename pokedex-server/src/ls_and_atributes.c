/*
 * ls_and_atributes.c
 *
 *  Created on: 13/10/2016
 *      Author: utnso
 */
#include "ls_and_atributes.h"
extern pthread_mutex_t mutex_por_archivo[];

/*---------------------------------------------TAMAÑO DE UN DIRECTORIO---------------------------------------------------*/
int tamanio_del_dir_raiz()
{
	return realizar_sumatoria_size_hijos("/");
}

int osada_b_calculate_size_of_directory(char *path_directory)
{
	if(string_equals_ignore_case(path_directory, "/"))
	{
		return realizar_sumatoria_size_hijos(path_directory);
	}
	else
	{
		if(es_directorio_vacio(path_directory))
		{
			return 0;
		}
		else
		{
			return realizar_sumatoria_size_hijos(path_directory);
		}
	}
}

int realizar_sumatoria_size_hijos(char *path)
{
	t_list *lista_hijos = osada_b_listar_hijos(path);
	int size = list_size(lista_hijos);
	int i;
	int sumatoria = 0;

	for(i=0; i<size;i++)
	{
		t_file_listado *file = list_get(lista_hijos,i);
		if(file->tipo == REGULAR)
		{
				sumatoria = sumatoria + file->tamanio;
		}
		else
		{

			sumatoria = sumatoria + osada_b_calculate_size_of_directory(file->path_completo);
		}
	}
	if(size==0)
	{
		list_destroy(lista_hijos);
	}
	else
	{
		list_destroy_and_destroy_elements(lista_hijos, file_listado_eliminate);
	}
	return sumatoria;
}

/*------------------------------------------LISTAR HIJOS  ----------------------------------------------------------------*/
t_list* osada_b_listar_hijos(char* path)
{
	t_list* lista = list_create();

	if(es_el_directorio_raiz(path))
	{
		listar_directorio_raiz(lista);
	}
	else
	{
		listar_directorio_comun(lista, path);
	}
	return lista;
}

void listar_directorio_raiz(t_list *lista)
{
	int index = 0;
	while(index<2048)
	{
		agregar_a_lista_si_es_hijo_de_raiz_full(index,lista);
		index++;
	}
}

void listar_directorio_comun(t_list *lista, char *path)
{
	t_info_file *info_file = dictionary_get(disco->diccionario_de_archivos,path);
	int index = 0;
	while(index<2048)
	{
		agregar_a_lista_si_es_hijo_full(info_file,index,lista);

		index++;
	}
}

int es_el_directorio_raiz(char *path)
{
	return string_equals_ignore_case(path,"/");
}

/*void agregar_a_lista_si_es_hijo(t_file_osada *path_padre, osada_file* hijo, t_list* lista)
{
	if(!verificar_si_son_mismo_files(path_padre->file,hijo))
	{
		if(es_el_padre(hijo, path_padre) && !verify_file_state(DELETED, hijo))
		{
			t_file_listado* dato_en_lista = malloc(sizeof(t_file_listado));
			char* path_hijo = crear_ruta((char*) hijo->fname,(char*)path_padre->path);

			dato_en_lista->path = path_hijo;
			dato_en_lista->tamanio =hijo->file_size;

			dato_en_lista->tipo = hijo->state;
			list_add(lista, dato_en_lista);
		}
	}

}*/

void agregar_a_lista_si_es_hijo_full(t_info_file *info_parent, int posicion_file_absoluto,t_list *lista)
{
	char* posicion_aux = string_itoa(posicion_file_absoluto);
	if(dictionary_has_key(disco->archivos_por_posicion_en_tabla_asig,posicion_aux))
	{
		t_info_file *info_file_a_check = dictionary_get(disco->archivos_por_posicion_en_tabla_asig,posicion_aux);
		free(posicion_aux);

		if(info_file_a_check->parent_block == info_parent->posicion_en_tabla_de_archivos)
			{
				osada_file *file = osada_get_file_for_index(info_file_a_check->posicion_en_tabla_de_archivos);
				t_file_listado* dato_en_lista = malloc(sizeof(t_file_listado));

				char* path_full = string_new();
				string_append(&path_full,info_file_a_check->path);
				dato_en_lista->path_completo = path_full;

				char *path_hijo = string_new();
				string_append(&path_hijo, "/");
				char* name_aux = modelar_nombre_archivo(file->fname);
				string_append(&path_hijo, name_aux);

				dato_en_lista->path = path_hijo;
				dato_en_lista->tamanio = file->file_size;

				dato_en_lista->tipo = file->state;
				list_add(lista, dato_en_lista);

				free(name_aux);
				free(file);
			}
	}
	else
	{
		free(posicion_aux);
	}
}

void agregar_a_lista_si_es_hijo_de_raiz(int num_raiz, osada_file *file, t_list *lista)
{
	if(num_raiz == file->parent_directory && !verify_file_state(DELETED,file) &&verify_correct_file(file))
	{
		t_file_listado* dato_en_lista = malloc(sizeof(t_file_listado));
		char *path_hijo = string_new();

		string_append(&path_hijo, "/");
		string_append(&path_hijo, (char*)file->fname);

		//dato_en_lista->file = osada_get_file_called(path_hijo,disco);
		dato_en_lista->path = path_hijo;
		dato_en_lista->tamanio = file->file_size;
		//dato_en_lista->tipo = dato_en_lista->file->file->state;
		dato_en_lista->tipo = file->state;
		list_add(lista, dato_en_lista);
	}
}

void agregar_a_lista_si_es_hijo_de_raiz_full(int numero_bloque_absoluto,t_list *lista)
{
	char* posicion_aux = string_itoa(numero_bloque_absoluto);
	if(dictionary_has_key(disco->archivos_por_posicion_en_tabla_asig,posicion_aux))
	{
		t_info_file *info_file = dictionary_get(disco->archivos_por_posicion_en_tabla_asig,posicion_aux);
		free(posicion_aux);

		if(info_file->parent_block == RAIZ)
		{
			osada_file *file = osada_get_file_for_index(info_file->posicion_en_tabla_de_archivos);
			t_file_listado* dato_en_lista = malloc(sizeof(t_file_listado));

			char* path_full = string_new();
			string_append(&path_full,info_file->path);
			dato_en_lista->path_completo = path_full;

			char *path_hijo = string_new();
			string_append(&path_hijo, "/");

			char* aux_name = modelar_nombre_archivo(file->fname);

			string_append(&path_hijo, aux_name);
			dato_en_lista->path = path_hijo;
			dato_en_lista->tamanio = file->file_size;
			dato_en_lista->tipo = file->state;
			list_add(lista, dato_en_lista);

			free(aux_name);
			free(file);
		}
	}
	else
	{
		free(posicion_aux);
	}
}


int es_el_padre(osada_file* file_hijo,t_file_osada *path_padre)
{
	int posicion_en_tabla_de_archivos = calcular_posicion_en_tabla_de_archivos(path_padre->block_relative,path_padre->position_in_block);
	return (posicion_en_tabla_de_archivos==file_hijo->parent_directory);
}

int verificar_si_son_mismo_files(osada_file *file_actual, osada_file *file_expected)
{
	int nombres_coinciden = string_equals_ignore_case((char*) file_actual->fname,(char*)file_expected->fname);
	return ((file_actual->parent_directory == file_expected->parent_directory) && nombres_coinciden);
}


t_list* listar_hijos_para_renombrar(int numero_padre)
{
	int i = 0;
	t_list *lista = list_create();

	while(i<2048)
	{
		char* posicion_aux = string_itoa(i);
		if(dictionary_has_key(disco->archivos_por_posicion_en_tabla_asig,posicion_aux))
		{
			t_info_file *info_file_a_check = dictionary_get(disco->archivos_por_posicion_en_tabla_asig,posicion_aux);
			free(posicion_aux);

			if(info_file_a_check->parent_block == numero_padre)
			{

				list_add(lista,info_file_a_check);
			}
		}
		else
		{
			free(posicion_aux);
		}
		i++;
	}
	return lista;
}

/*------------------------------------------ATRIBUTOS----------------------------------------------------------------------*/
enum
{
	ARCHIVO =1,
	DIRECTORIO =2
};

t_attributes_file* osada_b_get_attributes_of_this_file(char *path_file)
{
	t_info_file *info_file_central = dictionary_get(disco->diccionario_de_archivos,path_file);

	pthread_mutex_lock(&mutex_por_archivo[info_file_central->posicion_en_tabla_de_archivos]);

	osada_file *file = osada_get_file_for_index(info_file_central->posicion_en_tabla_de_archivos);
	t_attributes_file *atributos = malloc(sizeof(t_attributes_file));

	if(file->state == DIRECTORY)
	{
			//atributos->size = osada_b_calculate_size_of_directory(path_file);
			atributos->size = info_file_central->tamanio_del_directorio;
			atributos->tipo=DIRECTORIO;
	}
	else
	{
			atributos->size = file->file_size;
			atributos->tipo =ARCHIVO;
	}

	pthread_mutex_unlock(&mutex_por_archivo[info_file_central->posicion_en_tabla_de_archivos]);
	free(file);

	/*t_file_osada *file_find = osada_get_file_called(path_file, disco);

	//lock_file_full(file_find->block_relative + file_find->position_in_block);

	t_attributes_file *atributos = malloc(sizeof(t_attributes_file));

	if(file_find->file->state == DIRECTORY)
	{
		atributos->size = osada_b_calculate_size_of_directory(file_find->path);
		atributos->tipo=DIRECTORIO;
	}
	else
	{
		atributos->size = file_find->file->file_size;
		atributos->tipo =ARCHIVO;
	}

	//unlock_file_full(file_find->block_relative + file_find->position_in_block);

	t_file_osada_destroy(file_find);*/
	return atributos;
}
