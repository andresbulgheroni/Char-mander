/*
 * file_manipuling.c
 *
 *  Created on: 12/10/2016
 *      Author: utnso
 */
#include "file_manipuling.h"
extern pthread_mutex_t mutex_por_archivo[];
extern int *table_asignaciones;

/*------------------------------------------CREAR ARCHIVO------------------------------------------------------------------*/
t_osada_file_free* osada_b_file_create(int tipo,char* path)
{
	return osada_b_crear(tipo,path);
}

t_osada_file_free* osada_b_crear(int tipo, char* path)
{
	t_osada_file_free* archivo = osada_file_table_get_space_free(disco);

	char* path_padre = obtener_ruta_padre(path);

	if(!string_equals_ignore_case(path_padre,"/"))
	{
		t_info_file *info_padre = dictionary_get(disco->diccionario_de_archivos,path_padre);
		free(path_padre);

		char* nombre = array_last_element(path);
		setear_nombre(nombre,archivo->file);
		free(nombre);
		archivo->file->state = tipo;
		archivo->file->parent_directory = info_padre->posicion_en_tabla_de_archivos;
		archivo->file->file_size = 0;
		asignar_bloque_inicial_si_es_necesario(archivo->file, tipo);

		int posicion_absoluta_del_archivo = calcular_posicion_en_tabla_de_archivos_absoluta(archivo->block_relative-1,archivo->position_in_block);
		t_info_file *info_new = info_file_create(archivo->file,posicion_absoluta_del_archivo);
		info_new->parent_block=info_padre->posicion_en_tabla_de_archivos;
		info_new->posicion_en_tabla_de_archivos=posicion_absoluta_del_archivo;
		if(tipo==DIRECTORY)
		{
			info_new->tamanio_del_directorio = 0;
		}

		char* path_aux = string_new();
		string_append(&path_aux,path);

		info_new->path = string_new();
		string_append(&info_new->path,path_aux);

		dictionary_put(disco->diccionario_de_archivos,path_aux,info_new);
		char* pos_aux = string_itoa(posicion_absoluta_del_archivo);
		dictionary_put(disco->archivos_por_posicion_en_tabla_asig,pos_aux,info_new);

		free(path_aux);
		free(pos_aux);
	}
	else
	{
		free(path_padre);
		char* nombre = array_last_element(path);
		setear_nombre(nombre,archivo->file);
		free(nombre);
		archivo->file->state = tipo;
		archivo->file->parent_directory = RAIZ;
		archivo->file->file_size = 0;
		asignar_bloque_inicial_si_es_necesario(archivo->file, tipo);

		int posicion_absoluta_del_archivo = calcular_posicion_en_tabla_de_archivos_absoluta(archivo->block_relative-1,archivo->position_in_block);
		t_info_file *info_new = info_file_create(archivo->file,posicion_absoluta_del_archivo);
		info_new->parent_block=RAIZ;
		info_new->posicion_en_tabla_de_archivos=posicion_absoluta_del_archivo;
		if(tipo==DIRECTORY)
		{
			info_new->tamanio_del_directorio = 0;
		}

		char* path_aux = string_new();
		string_append(&path_aux,path);

		info_new->path =string_new();
		string_append(&info_new->path,path_aux);

		dictionary_put(disco->diccionario_de_archivos,path_aux,info_new);
		char* pos_aux = string_itoa(posicion_absoluta_del_archivo);
		dictionary_put(disco->archivos_por_posicion_en_tabla_asig,pos_aux,info_new);

		free(path_aux);
		free(pos_aux);

	}
	return archivo;
}


int calcular_posicion_en_tabla_de_archivos_absoluta(int bloque, int posicion_dentro_del_bloque)
{
	if(bloque==0)
	{
		if(posicion_dentro_del_bloque==0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		if(posicion_dentro_del_bloque==0)
		{
			return (bloque*2);
		}
		else
		{
			return (bloque*2) +1;
		}
	}
}


void asignar_bloque_inicial_si_es_necesario(osada_file *file, int tipo)
{
	if(tipo == DIRECTORY)
	{
		file->first_block = FEOF;

	}
	else
	{
		file->first_block = FEOF;
	}
}


void setear_nombre(char* nombre, osada_file* archivo)
{
	int i = 0;
	int size_name = string_length(nombre);
	while(i<size_name)
	{
		archivo->fname[i] = nombre[i];
		i++;
	}
	if(size_name!=17)
	{
		archivo->fname[size_name] = '\0';
	}
}


void setear_bloque_padre(osada_file *file, char *path)
{
	char *path_padre = obtener_ruta_padre(path);
	if(string_equals_ignore_case(path_padre,"/"))
	{
		file->parent_directory = RAIZ;
	}
	else
	{
		t_file_osada *file_padre = osada_get_file_called(path_padre,disco);
		int bloque_padre = calcular_posicion_en_tabla_de_archivos(file_padre->block_relative,file_padre->position_in_block);
		file->parent_directory = bloque_padre;
		t_file_osada_destroy(file_padre);
	}

}


int osada_b_get_a_new_block_init()
{
	int primer_bloque_absoluto = osada_ocupa_bit_libre_de(disco);
	int primer_bloque_relativo = calcular_posicion_relativa_en_bloque_de_datos(primer_bloque_absoluto);
	limpiar_bloque_de_datos(primer_bloque_relativo);
	return primer_bloque_relativo;
}


void limpiar_bloque_de_datos(int n)
{
	char* aux = malloc(OSADA_BLOCK_SIZE);
	int i;
	for(i=0;i<OSADA_BLOCK_SIZE;i++)
	{
		aux[i]='\0';
	}
	osada_push_block(BLOQUE_DE_DATOS,n,aux,disco);
	free(aux);
}


int osada_b_check_repeat_name(int tipo,char* path)
{
	if(!osada_check_exists_in_dictionary(path))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}


char* obtener_nuevo_path(char* old_path, char* new_name)
{
	char *path_padre = obtener_ruta_padre(old_path);
	string_append(&path_padre, "/");
	string_append(&path_padre, new_name);
	return path_padre;
}

/*----------------------------------------------OBTENCION DE NUM BLOQUES ARCHIVO-------------------------------------------*/
t_list* osada_get_blocks_nums_of_this_file_Asco(osada_file *file, t_disco_osada *disco)
{
	t_list *list_blocks = list_create();
	osada_block_pointer before_block = file->first_block;
	osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);

	int* primer_bloque = malloc(sizeof(int));
	*primer_bloque = file->first_block;

	list_add(list_blocks,primer_bloque);

	int hay_mas_para_leer = 1;
	while(hay_mas_para_leer)
	{
		int *block = osada_get_bytes_start_in(byte_inicial_tabla_asignaciones + 4*before_block,sizeof(int),disco->map);
		if(*block == FEOF)
		{
			hay_mas_para_leer=0;
			free(block);
		}
		else
		{
			list_add(list_blocks, block);
			before_block = *block;
		}
	}
	return list_blocks;
}

t_list* osada_get_blocks_nums_of_this_file(osada_file *file, t_disco_osada *disco)
{
	t_list *list_blocks = list_create();
	osada_block_pointer before_block = file->first_block;


	int* primer_bloque = malloc(sizeof(int));
	*primer_bloque = file->first_block;

	list_add(list_blocks,primer_bloque);

	int hay_mas_para_leer = 1;
	while(hay_mas_para_leer)
	{
		int *block = malloc(sizeof(int));
		 *block = table_asignaciones[before_block];
		if(*block == FEOF)
		{
			hay_mas_para_leer=0;
			free(block);
		}
		else
		{
			list_add(list_blocks, block);
			before_block = *block;
		}
	}
	return list_blocks;
}

t_list* osada_get_blocks_nums_of_this_file_since_full(int start_block)
{
	t_list *list_blocks = list_create();
	osada_block_pointer before_block = start_block;
	//osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);

	int* primer_bloque = malloc(sizeof(int));
	*primer_bloque = start_block;

	list_add(list_blocks,primer_bloque);

	int hay_mas_para_leer = 1;
	while(hay_mas_para_leer)
	{
		int *block = malloc(sizeof(int));
		*block = table_asignaciones[before_block];
		if(*block == FEOF)
		{
			hay_mas_para_leer=0;
			free(block);
		}
		else
		{
			list_add(list_blocks, block);
			before_block = *block;
		}
	}
	return list_blocks;
}


t_list* osada_get_blocks_nums_of_this_file_since(int start_block)
{
	t_list *list_blocks = list_create();
	osada_block_pointer before_block = start_block;
	osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);

	int* primer_bloque = malloc(sizeof(int));
	*primer_bloque = start_block;

	list_add(list_blocks,primer_bloque);

	int hay_mas_para_leer = 1;
	while(hay_mas_para_leer)
	{
		int *block = osada_get_bytes_start_in(byte_inicial_tabla_asignaciones + 4*before_block,sizeof(int),disco->map);
		if(*block == FEOF)
		{
			hay_mas_para_leer=0;
			free(block);
		}
		else
		{
			list_add(list_blocks, block);
			before_block = *block;
		}
	}
	return list_blocks;
}

/*---------------------------------------------BUSUQEDA DE TABLA DE ARCHIVOS DISPONIBLE------------------------------------*/
t_osada_file_free* osada_file_table_get_space_free(t_disco_osada *disco)
{
	int index = 1;

	osada_file *file_1 = malloc(sizeof(osada_file));
	osada_file *file_2 = malloc(sizeof(osada_file));

	int file_free = 0;

	t_osada_file_free *a_file_free = malloc(sizeof(t_osada_file_free));

	while(!file_free)
	{
		void *two_files = osada_get_blocks_relative_since(TABLA_DE_ARCHIVOS,index,1,disco);
		memcpy(file_1,two_files, sizeof(osada_file));
		memcpy(file_2,two_files + sizeof(osada_file), sizeof(osada_file));

		if(verify_file_state(DELETED,file_1))
		{
				file_free = 1;
				//free(two_files);
				free(file_2);
				a_file_free->file = file_1;
				a_file_free->block_relative = index;
				a_file_free->position_in_block = 0;
		}
		else
		{
			if(verify_file_state(DELETED,file_2))
			{
				file_free = 1;
				//free(two_files);
				free(file_1);
				a_file_free->file = file_2;
				a_file_free->block_relative = index;
				a_file_free->position_in_block = 32;
			}
		}
		free(two_files);
		index++;
	}
	return a_file_free;
}


int osada_check_is_table_asig_is_full()
{
		osada_file *file_1 = malloc(sizeof(osada_file));
		osada_file *file_2 = malloc(sizeof(osada_file));

		int file_free = 0;
		t_osada_file_free *a_file_free = malloc(sizeof(t_osada_file_free));

		int index = 1;
		while(!file_free && index<=2048)
		{
			void *two_files = osada_get_blocks_relative_since(TABLA_DE_ARCHIVOS,index,1,disco);
			memcpy(file_1,two_files, sizeof(osada_file));
			memcpy(file_2,two_files + sizeof(osada_file), sizeof(osada_file));
			if(verify_file_state(DELETED,file_1))
			{
					file_free = 1;
			}
			else
			{
				if(verify_file_state(DELETED,file_2))
				{
					file_free = 1;
				}
			}
			free(two_files);
			index++;
		}
		free(a_file_free);
		free(file_1);
		free(file_2);
		if(file_free==0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
}


int verify_file_state(int state,osada_file *file)
{
	if(file->state == state)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


int verify_correct_file(osada_file *file)
{
	if(file->state == REGULAR || file->state==DIRECTORY || file->state==DELETED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*---------------------------------------------VERIFICACION EXISTENCIA DE UN PATH------------------------------------------*/
int osada_check_exist(char *path)
{
	int verify = osada_check_exists_in_dictionary(path);
	return verify;
}

int verificar_existencia(char *file_or_directory, uint16_t dad_block)
{
	void *result = osada_get_file_called(file_or_directory, disco);


	t_file_osada *file = (t_file_osada *) result;

	if(!existe_posta(result))
	{
		return -1;
	}
	else
	{

		int satisfy = -1;

		if(dad_block == RAIZ)
		{
			satisfy = calcular_posicion_en_tabla_de_archivos(file->block_relative,file->position_in_block);
			t_file_osada_destroy(file);
			return satisfy;
		}
		else
		{
			if(file->file->parent_directory == dad_block)
			{
				satisfy = calcular_posicion_en_tabla_de_archivos(file->block_relative, file->position_in_block);
				t_file_osada_destroy(file);
				return satisfy;
			}
			else
			{
				t_file_osada_destroy(file);
				return -1;
			}
		}

	}
}

int existe_posta(void* result)
{

	t_file_osada *file = (t_file_osada *) result;

	if(!string_equals_ignore_case((char*) result, "NO_EXISTE"))
	{
		if(verify_file_state(DELETED,file->file))
		{
			t_file_osada_destroy(file);
			return 0;
		}
		else
		{
			return 1;
		}

	}
	else
	{
		free(result);
		return 0;
	}
}

int revisar_resultado(int result)
{
	if(result == -1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int calcular_posicion_en_tabla_de_archivos(int num_block, int position)
{
	switch(num_block)
	{
		case(1):
		{
			return position;
		};break;
		case(2):
		{
			return position + 2;
		};break;
		default:
		{
			if(position == 0)
			{
				return (num_block + num_block -2);
			}
			else
			{
				return (num_block + num_block -1);
			}
		}
	}
}

int calcular_bloque_en_tabla_de_archivos_segun_parent_directory(int parent)
{
	switch(parent)
	{
		case(0): return 1;
		case(1): return 1;
		default:
		{
			if(es_par(parent))
			{
				return((parent/2) +1);
			}
			else
			{
				return (parent - ((parent -1) - ((parent -1)/2)));
			}
		}
	}
}
/*----------------------------------------------OBTENCION DE UN ARCHIVO ESPECIFICO----------------------------------------*/
void* osada_get_file_called(char *path, t_disco_osada *disco)
{
	int archivo_encontrado = 0;
	int archivo;
	int index = 1;

	osada_file *file_1 = malloc(sizeof(osada_file));
	osada_file *file_2 = malloc(sizeof(osada_file));

	//pthread_mutex_lock(&mutex_operaciones);
	while(!archivo_encontrado && index<=1024)
	{
		void *two_files = osada_get_blocks_relative_since(TABLA_DE_ARCHIVOS,index,1,disco);
		memcpy(file_1,two_files, sizeof(osada_file));
		memcpy(file_2,two_files + sizeof(osada_file), sizeof(osada_file));
		if(verificar_si_es_archivo_buscado(path, file_1))
		{
			archivo_encontrado = 1;
			archivo = 1;
			free(two_files);
		}
		else
		{
			if(verificar_si_es_archivo_buscado(path,file_2))
			{
				archivo_encontrado = 1;
				archivo = 2;
				free(two_files);
			}
			else
			{
				free(two_files);
			}
		}
		index++;
	}
	//pthread_mutex_unlock(&mutex_operaciones);
	if(archivo_encontrado == 0)
	{
		free(file_1);
		free(file_2);
		char* error_no = string_new();
		string_append(&error_no,"NO_EXISTE");
		return error_no;
	}
	else
	{
		t_file_osada *file = malloc(sizeof(t_file_osada));
		switch(archivo)
		{
			case(1):
			{
				free(file_2);
				file->file = file_1;
				file->block_relative = index -1;
				file->position_in_block = 0;
				file->path = path;
				return file;
			}
			case(2):
			{
				free(file_1);
				file->file = file_2;
				file->block_relative = index -1;
				file->position_in_block = 1;
				file->path = path;
				return file;
			}
		}
	}

}

int verificar_si_es_archivo_buscado(char *path, osada_file *file)
{
	if(verificar_si_es_raiz(path))
	{
		char *name = path_first_reference(path);
		int check;
		if(string_length(name) == OSADA_FILENAME_LENGTH)
		{
			check = string_equals_osada_max_lenght(name,file->fname);
		}
		else
		{
			check =string_equals_ignore_case(name, (char*)file->fname);
		}
		free(name);
		return ( check && (file->parent_directory == RAIZ));
	}
	else
	{
		return comprobar_igualdad(path, file);
	}
}

int string_equals_osada_max_lenght(char* name, unsigned char* name_2)
{
	int verifica = 1;
	int i=0;

	while(i<OSADA_FILENAME_LENGTH && verifica)
	{
		if(name_2[i]!=name[i])
		{
			verifica = 0;
		}
		i++;
	}
	return verifica;
}

int comprobar_igualdad(char *path, osada_file *file)
{
	char *ultimo_elemento_path = array_last_element(path);
	if(verificar_si_nombre_coincide(ultimo_elemento_path, file->fname))
	{
		free(ultimo_elemento_path);
		return osada_b_check_parents(path, file);
	}

	else
	{
		free(ultimo_elemento_path);
		return 0;
	}
}

int osada_b_check_parents(char *path, osada_file *file)
{
	char** file_for_file = string_split(path, "/");
	int size = array_size(file_for_file);
	int i = size -2;

	osada_file *file_1 = malloc(sizeof(osada_file));
	osada_file *file_2 = malloc(sizeof(osada_file));

	int comprueba = 1;
	int parent=file->parent_directory;

	while(comprueba && i>=0)
	{	if(parent != RAIZ)
		{
			int posicion_en_array = calcular_bloque_en_tabla_de_archivos_segun_parent_directory(parent);
			void* two_files = osada_get_blocks_relative_since(TABLA_DE_ARCHIVOS, posicion_en_array,1,disco);
			memcpy(file_1,two_files, sizeof(osada_file));
			memcpy(file_2,two_files + sizeof(osada_file), sizeof(osada_file));

			if(es_par(parent))
			{
				if(verificar_si_nombre_coincide(file_for_file[i],file_1->fname))
					{
						parent = file_1->parent_directory;
					}
					else
					{
					comprueba = 0;
					}
			}
			else
				{
					if(verificar_si_nombre_coincide(file_for_file[i],file_2->fname))
					{
						parent = file_2->parent_directory;
					}
					else{comprueba = 0;}
				}

			i--;
		}
	else
	{
		comprueba =0;
	}

	}

	free(file_1);
	free(file_2);
	array_free_all(file_for_file);
	return comprueba;
}

int es_par(int numero)
{
	return (numero % 2 == 0 );
}

int verificar_si_nombre_coincide(char *path, unsigned char* file_name)
{
	if(string_length(path) == OSADA_FILENAME_LENGTH)
	{
		return string_equals_osada_max_lenght(path,file_name);
	}
	else
	{
		return string_equals_ignore_case(path, (char*)file_name);
	}

}

char* path_first_reference(char *path)
{
	char **file_for_file = string_split(path,"/");
	char *first = string_new();
	string_append(&first,file_for_file[0]);
	array_free_all(file_for_file);
	return first;
}

char* path_delete_last_reference(char *path_init)
{
	char **file_for_file = string_split(path_init,"/");
	int size = array_size(file_for_file);
	int i = 0;

	char *path_new = string_new();
	string_append(&path_new, "/");
	while(i < (size-2))
	{
		string_append(&path_new, file_for_file[i]);
		string_append(&path_new, "/");
		i++;
	}
	return path_new;
}

int verificar_si_es_raiz(char *path)
{
	char **file_for_file = string_split(path,"/");
	int size = array_size(file_for_file);
	if(size == 1)
	{
		array_free_all(file_for_file);
		return 1;
	}
	else
	{
		array_free_all(file_for_file);
		return 0;
	}
}

char* obtener_ruta_padre(char* path)
{
	char** por_separado = string_split(path, "/");
	int size = array_size(por_separado);
	if(size == 1)
	{
		array_free_all(por_separado);
		char *raiz = string_new();
		string_append(&raiz,"/");
		return raiz;
	}
	else
	{
		int i;
		char *path_padre = string_new();

		for(i=0;i<size-1;i++)
			{
				string_append(&path_padre,"/");
				string_append(&path_padre, por_separado[i]);
			}
		array_free_all(por_separado);
		return path_padre;
	}
}

char* obtener_ruta_especifica(char *ruta_inicial, char *directorio_o_nombre_archivo, char *sub_directorio_o_nombre_archivo)
{
	char* ruta = string_new();
	string_append(&ruta,ruta_inicial);
	string_append(&ruta, "/");
	string_append(&ruta, directorio_o_nombre_archivo);
	if(sub_directorio_o_nombre_archivo != NULL)
	{	string_append(&ruta, "/");
		string_append(&ruta,sub_directorio_o_nombre_archivo);
		string_trim_left(&ruta);
		return ruta;
	}
	else
		{
			string_trim_left(&ruta);
			return ruta;
		}
}

/*----------------------------------------------RENAME----------------------------------------------------------------------*/
void osada_b_rename(t_file_osada *file, char* new_path)
{
	char *new_nombre = array_last_element(new_path);
	setear_nombre(new_nombre,file->file);
	int offset = calcular_desplazamiento_tabla_de_archivos(file->position_in_block);
	//pthread_mutex_lock(&mutex_operaciones);
	osada_push_middle_block(TABLA_DE_ARCHIVOS,file->block_relative,offset,file->file,disco);
	//pthread_mutex_unlock(&mutex_operaciones);
	free(new_nombre);
}


void osada_b_rename_full(t_to_be_rename *to_be_rename)
{
	t_info_file* info_file = dictionary_remove(disco->diccionario_de_archivos,to_be_rename->old_path);
	pthread_mutex_lock(&mutex_por_archivo[info_file->posicion_en_tabla_de_archivos]);

	free(info_file->path);

	char* aux = string_new();
	string_append(&aux,to_be_rename->new_path);

	info_file->path=aux;

	dictionary_put(disco->diccionario_de_archivos,aux,info_file);

	osada_file *file = osada_get_file_for_index(info_file->posicion_en_tabla_de_archivos);
	if(file->state == DIRECTORY)
	{
		renombrar_path_hijos(info_file,to_be_rename->new_path);
	}

	char *new_nombre = array_last_element(to_be_rename->new_path);
	setear_nombre(new_nombre,file);
	free(new_nombre);
	osada_impactar_un_archivo(info_file->posicion_en_tabla_de_archivos,file);
	free(file);
	pthread_mutex_unlock(&mutex_por_archivo[info_file->posicion_en_tabla_de_archivos]);
}

void renombrar_path_hijos(t_info_file *info_file_a_renombrar, char* new_path)
{
	t_list *hijos = listar_hijos_para_renombrar(info_file_a_renombrar->posicion_en_tabla_de_archivos);
	int size = list_size(hijos);
	if(size!=0)
	{
		int i=0;
		while(i<size)
		{
			t_info_file *info_hijo = list_get(hijos,i);
			t_info_file *info_aux = dictionary_remove(disco->diccionario_de_archivos,info_hijo->path);

			char* aux_new_path = string_new();
			string_append(&aux_new_path,new_path);
			string_append(&aux_new_path,"/");
			char* nombre_posta_del_archivo = array_last_element(info_hijo->path);
			string_append(&aux_new_path,nombre_posta_del_archivo);

			free(nombre_posta_del_archivo);
			free(info_hijo->path);

			info_hijo->path = string_new();
			string_append(&info_hijo->path,aux_new_path);

			dictionary_put(disco->diccionario_de_archivos,aux_new_path,info_hijo);

			free(aux_new_path);

			osada_file *file_hijo = osada_get_file_for_index(info_hijo->posicion_en_tabla_de_archivos);
			if(file_hijo->state == DIRECTORY)
			{
				renombrar_path_hijos(info_hijo,info_hijo->path);
			}
			free(file_hijo);

			i++;
		}
	}
	else
	{
		list_destroy(hijos);
	}
}

int osada_b_check_name(char* path)
{
	char *name = array_last_element(path);
	int size = string_length(name);
	free(name);
	if(size >OSADA_FILENAME_LENGTH){
		return 0;
	}
	else
	{
		return 1;
	}
}
