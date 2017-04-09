/*
 * osada_generales.c
 *
 *  Created on: 12/10/2016
 *      Author: utnso
 */

#include "osada_generales.h"
pthread_mutex_t mutex_operaciones= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_por_archivo[2048];
pthread_mutex_t mutex_por_archivo_borrado[2048];
int *table_asignaciones;
int primer_bloque_libre_para_asignar;

int primer_bloque_libre();

t_disco_osada* osada_disco_abrite(char *ruta)
{
	t_disco_osada *disco_new = malloc(sizeof(t_disco_osada));
	disco_new->ruta = ruta;
	disco_new->size = disco_dame_tu_tamanio(ruta);
	disco_new->file_descriptor = disco_dame_tu_descriptor(ruta);
	disco_new->map = disco_dame_mapping(disco_new->size, disco_new->file_descriptor);
	disco_new->header = osada_header_create(disco_new->map);
	disco_new->bitmap = osada_bitmap_create(disco_new);
	disco_new->diccionario_de_archivos = dictionary_create();
	disco_new->archivos_por_posicion_en_tabla_asig = dictionary_create();
	table_asignaciones = malloc(sizeof(int) * (calcular_tamanio_tabla_de_asignaciones(disco_new->header)*OSADA_BLOCK_SIZE));
	primer_bloque_libre_para_asignar = 0;
	return disco_new;
}

void iniciar_semaforos()
{
	int index;
	for(index =0;index<2048;index++)
	{
		pthread_mutex_init(&mutex_por_archivo[index],NULL);
		pthread_mutex_init(&mutex_por_archivo_borrado[index],NULL);
	}
}

void lock_file_full(int num_block_file, int offset)
{
	int resultado = 2*num_block_file + offset;
	pthread_mutex_lock(&mutex_por_archivo[resultado]);
}

void unlock_file_full(int num_block_file,int offset)
{
	int resultado = 2*num_block_file + offset;
	pthread_mutex_unlock(&mutex_por_archivo[resultado]);
}

void lock_file_to_delte(int num_block_file,int offset)
{
	int resultado = 2*num_block_file + offset;
	pthread_mutex_lock(&mutex_por_archivo_borrado[resultado]);
}

void unlock_file_to_delte(int num_block_file,int offset)
{
	int resultado = 2*num_block_file + offset;
	pthread_mutex_unlock(&mutex_por_archivo_borrado[resultado]);
}

/*----------------------------------------------------RECUPERANDO TABLA DE ASIGNACIONES------------------------------------*/

void recuperar_tabla_de_asignaciones()
{
	int i = 0;
	while(i<2048)
	{
		char* pos_aux = string_itoa(i);
		if(dictionary_has_key(disco->archivos_por_posicion_en_tabla_asig,pos_aux))
		{
			recuperar_bloques_asigados_array(i);
		}
		free(pos_aux);
		i++;
	}
	if(disco->cantidad_bloques_libres >0)
	{
		primer_bloque_libre_para_asignar = primer_bloque_libre();
	}
	else
	{
		primer_bloque_libre_para_asignar = FEOF;
	}
}

void recuperar_bloques_asigados(int posicion)
{
	osada_file *file = osada_get_file_for_index(posicion);
	int tipo = file->state;
	osada_block_pointer before_block = file->first_block;
	free(file);
	if(tipo == REGULAR)
	{
		osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);
		if(before_block != FEOF)
		{
			int hay_mas_para_leer = 1;
			while(hay_mas_para_leer)
			{
				int *block = osada_get_bytes_start_in(byte_inicial_tabla_asignaciones + 4*before_block,sizeof(int),disco->map);
				if(*block == FEOF)
					{
						//table_asignaciones[before_block] = FEOF;
						hay_mas_para_leer=0;
						free(block);
					}
				else
					{
						char* pos_aux = string_itoa(before_block);
						int  *info = dictionary_get(disco->table_asig,pos_aux);
						*info = *block;
						free(pos_aux);
						before_block = *block;
						free(block);
					}
			}
		}
	}
}

void recuperar_bloques_asigados_array(int posicion)
{
	osada_file *file = osada_get_file_for_index(posicion);
	int tipo = file->state;
	osada_block_pointer before_block = file->first_block;
	free(file);
	if(tipo == REGULAR)
	{
		osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);
		if(before_block != FEOF)
		{
			int hay_mas_para_leer = 1;
			controlar_en_bitarray(before_block);
			while(hay_mas_para_leer)
			{
				int *block = osada_get_bytes_start_in(byte_inicial_tabla_asignaciones + 4*before_block,sizeof(int),disco->map);
				if(*block == FEOF)
					{
						table_asignaciones[before_block] = FEOF;
						hay_mas_para_leer=0;
						free(block);
					}
				else
					{

						table_asignaciones[before_block] = *block;
						before_block = *block;
						controlar_en_bitarray(before_block);
						free(block);
					}
			}
		}
	}
}

void controlar_en_bitarray(int block_relative_data)
{
	int bloque_incial = osada_get_start_block_absolut_of(BLOQUE_DE_DATOS,disco);
	int result = bitarray_test_bit(disco->bitmap,bloque_incial+ block_relative_data);
	if(result != 1)
	{
		bitarray_set_bit(disco->bitmap,bloque_incial+block_relative_data);
		osada_disminui_cantidad_bloques_libres(1);
	}
}

void limpiar_table_asignaciones()
{
	int tamanio = calcular_tamanio_tabla_de_asignaciones(disco->header)*OSADA_BLOCK_SIZE;
	int i;
	for(i=0;i<tamanio;i++)
	{
		table_asignaciones[i] = FEOF;
	}
}

int buscar_bloque_numero_array(osada_file *file,int numero_bloque_a_recuperar)
{
	if(numero_bloque_a_recuperar == 0)
	{
		return file->first_block;
	}
	else
	{
		int i=0;
		osada_block_pointer before_block = file->first_block;
		while(i<numero_bloque_a_recuperar)
		{
			char* aux = string_itoa(before_block);
			int* block_number = dictionary_get(disco->table_asig,aux);
			before_block = *block_number;
			free(aux);
			i++;
		}
		return before_block;
	}
}

int buscar_bloque_numero(osada_file *file,int numero_bloque_a_recuperar)
{
	if(numero_bloque_a_recuperar == 0)
	{
		return file->first_block;
	}
	else
	{
		int i=0;
		osada_block_pointer before_block = file->first_block;
		while(i<numero_bloque_a_recuperar)
		{
			before_block = table_asignaciones[before_block];
			i++;
		}
		return before_block;
	}
}

void settear_valor_en_tabla_asginaciones(int posicion, int value)
{
	pthread_mutex_lock(&mutex_operaciones);
	table_asignaciones[posicion]=value;
	pthread_mutex_unlock(&mutex_operaciones);
}

/*----------------------------------------------------RECUPERANDO ARCHIVOS A DICCIONARIO------------------------------------*/
void disco_recupera_tus_archivos()
{
	disco_recupera_archivos_raiz(disco->diccionario_de_archivos,disco->archivos_por_posicion_en_tabla_asig);
	disco_recupera_arbolada_de_archivos(disco->diccionario_de_archivos);
	recuperar_tamanio_de_directorios();
	agregar_y_recuperar_dir_raiz();
}

void disco_recupera_archivos_raiz(t_dictionary *diccionario_key_por_path, t_dictionary *diccionario_key_por_posicion)
{
	int index = 1;
	osada_file *file_1 = malloc(sizeof(osada_file));
	osada_file *file_2 = malloc(sizeof(osada_file));
	while(index<=1024)
	{
		void *two_files = osada_get_blocks_relative_since(TABLA_DE_ARCHIVOS,index,1,disco);
		memcpy(file_1,two_files, sizeof(osada_file));
		memcpy(file_2,two_files + sizeof(osada_file), sizeof(osada_file));

		int posicion_archivo_1 = (index-1)*2;
		int posicion_archivo_2 =(index-1)*2 +1;

		agregar_a_diccionario_si_es_necesario(file_1,posicion_archivo_1,diccionario_key_por_path,diccionario_key_por_posicion);
		agregar_a_diccionario_si_es_necesario(file_2,posicion_archivo_2,diccionario_key_por_path,diccionario_key_por_posicion);

		free(two_files);
		index++;
	}
	free(file_1);
	free(file_2);
}

void agregar_a_diccionario_si_es_necesario(osada_file *file,int posicion,t_dictionary *diccionario_por_path, t_dictionary *diccionario_por_posicion)
{
	if(es_archivo_raiz(file) )
	{
		char* path = string_new();
		string_append(&path,"/");
		char* name_file_aux = modelar_nombre_archivo(file->fname);
		string_append(&path,name_file_aux);
		free(name_file_aux);

		t_info_file *new_info_file = info_file_create(file, posicion);
		new_info_file->path = string_new();
		string_append(&new_info_file->path,path);
		new_info_file->parent_block=RAIZ;
		dictionary_put(diccionario_por_path,path,new_info_file);
		char* posicion_string = string_itoa(posicion);
		dictionary_put(diccionario_por_posicion,posicion_string,new_info_file);

		free(path);
		free(posicion_string);
	}
}

t_info_file* info_file_create(osada_file *file, int posicion)
{
	t_info_file *info_new = malloc(sizeof(t_info_file));
	info_new->posicion_en_tabla_de_archivos = posicion;
	if(verify_file_state(REGULAR,file))
	{
		if(file->first_block== FEOF)
		{
			info_new->last_block_asigned = FEOF;
			info_new->last_block_write=FEOF;
			info_new->cantidad_bloques_asignados = 0;
		}
		else
		{
			t_list *bloques = osada_get_blocks_nums_of_this_file_Asco(file,disco);
			int tamanio_lista = list_size(bloques);

			int *ultimo_bloque_asignado = list_get(bloques,tamanio_lista-1);
			info_new->last_block_asigned=*ultimo_bloque_asignado;
			info_new->last_block_write=*ultimo_bloque_asignado;
			info_new->cantidad_bloques_asignados = tamanio_lista;
			list_destroy_and_destroy_elements(bloques,free_list_blocks);

		}
	}
	else
	{
		info_new->last_block_asigned=FEOF;
		info_new->last_block_write=FEOF;
	}


	return info_new;

}

int es_archivo_raiz(osada_file *file)
{
	if(RAIZ == file->parent_directory && !verify_file_state(DELETED,file) &&verify_correct_file(file))
	{
			return 1;
	}
	else
	{
		return 0;
	}
}

void disco_recupera_arbolada_de_archivos(t_dictionary *diccionario)
{

	int index = 1;
	osada_file *file_1 = malloc(sizeof(osada_file));
	osada_file *file_2 = malloc(sizeof(osada_file));
	while(index<=1024)
	{
		void *two_files = osada_get_blocks_relative_since(TABLA_DE_ARCHIVOS,index,1,disco);
		memcpy(file_1,two_files, sizeof(osada_file));
		memcpy(file_2,two_files + sizeof(osada_file), sizeof(osada_file));

		int posicion_archivo_1 = (index-1)*2;
		int posicion_archivo_2 =(index-1)*2 +1;
		agregar_a_diccionario_como_arbolada_si_es_necesario(file_1,posicion_archivo_1,diccionario);
		agregar_a_diccionario_como_arbolada_si_es_necesario(file_2,posicion_archivo_2,diccionario);

		free(two_files);
		index++;
	}
	free(file_1);
	free(file_2);
}

void agregar_a_diccionario_como_arbolada_si_es_necesario(osada_file *file, int posicion, t_dictionary *diccionario)
{
	if(RAIZ != file->parent_directory && !verify_file_state(DELETED,file) &&verify_correct_file(file))
	{	char* aux_parent_directory = string_itoa(file->parent_directory);
		if(dictionary_has_key(disco->archivos_por_posicion_en_tabla_asig,aux_parent_directory))
		{
			t_info_file *info_padre = dictionary_get(disco->archivos_por_posicion_en_tabla_asig,aux_parent_directory);

			char* aux_path_new = string_new();

			string_append(&aux_path_new,info_padre->path);
			string_append(&aux_path_new,"/");

			char* name_file_aux = modelar_nombre_archivo(file->fname);

			string_append(&aux_path_new,name_file_aux);
			free(name_file_aux);


			t_info_file *new_info_file = info_file_create(file, posicion);
			new_info_file->path = string_new();
			string_append(&new_info_file->path,aux_path_new);

			new_info_file->parent_block = info_padre->posicion_en_tabla_de_archivos;
			dictionary_put(diccionario,aux_path_new,new_info_file);
			char* posicion_string = string_itoa(posicion);
			dictionary_put(disco->archivos_por_posicion_en_tabla_asig,posicion_string,new_info_file);

			free(aux_parent_directory);
			free(aux_path_new);
			free(posicion_string);
		}
		else
		{
			free(aux_parent_directory);
		}
	}
}

void recuperar_tamanio_de_directorios()
{
	int index=0;
	while(index<2048)
	{
		char* aux = string_itoa(index);
		if(dictionary_has_key(disco->archivos_por_posicion_en_tabla_asig,aux))
		{
			osada_file* file = osada_get_file_for_index(index);
			if(file->state == DIRECTORY)
			{
				t_info_file *info = dictionary_get(disco->archivos_por_posicion_en_tabla_asig,aux);
				info->tamanio_del_directorio = osada_b_calculate_size_of_directory(info->path);
			}
			free(file);
		}
		free(aux);
		index++;
	}
}

void agregar_y_recuperar_dir_raiz()
{
	t_info_file *info_raiz = malloc(sizeof(t_info_file));
	info_raiz->path = string_new();
	string_append(&info_raiz->path,"/");
	info_raiz->tamanio_del_directorio = tamanio_del_dir_raiz();
	dictionary_put(disco->diccionario_de_archivos, "/",info_raiz);

}

void actualizar_tamanio_del_padre(t_info_file *info, int size_a_sumar)
{
	if(info->parent_block == RAIZ)
	{
		pthread_mutex_lock(&mutex_operaciones);
		t_info_file *info_raiz = dictionary_get(disco->diccionario_de_archivos,"/");
		info_raiz->tamanio_del_directorio = info_raiz->tamanio_del_directorio +size_a_sumar; // REVISAR ACA
		pthread_mutex_unlock(&mutex_operaciones);
	}
	else
	{
		pthread_mutex_lock(&mutex_operaciones);
		char* aux = string_itoa(info->parent_block);
		t_info_file *info_padre  = dictionary_get(disco->archivos_por_posicion_en_tabla_asig,aux);
		free(aux);
		info_padre->tamanio_del_directorio = info_padre->tamanio_del_directorio + size_a_sumar;
		pthread_mutex_unlock(&mutex_operaciones);
	}
}

/*-------------------------------------------------------CREATES Y RECUPEROS-----------------------------------------------*/
int disco_recupera_cantidad_bloques_libres()
{
	int bloque_incial = osada_get_start_block_absolut_of(BLOQUE_DE_DATOS,disco);
	int i =bloque_incial;
	int contador = 0;
	while( i<(bloque_incial + disco->header->data_blocks))
	{
		if(!bitarray_test_bit(disco->bitmap,i))
		{
				contador++;
		}
		i++;
	}
	return contador;
}

void osada_aumenta_cantidad_bloques_libres(int n)
{
	pthread_mutex_lock(&mutex_operaciones);
	disco->cantidad_bloques_libres = disco->cantidad_bloques_libres +n;
	pthread_mutex_unlock(&mutex_operaciones);
}

void osada_disminui_cantidad_bloques_libres(int n)
{
	//pthread_mutex_lock(&mutex_operaciones);
	disco->cantidad_bloques_libres = disco->cantidad_bloques_libres -n;
	//pthread_mutex_unlock(&mutex_operaciones);
}

int disco_recupera_cantidad_Archivos()
{
	osada_file *file_1 = malloc(sizeof(osada_file));
	osada_file *file_2 = malloc(sizeof(osada_file));

	int count = 0;
	t_osada_file_free *a_file_free = malloc(sizeof(t_osada_file_free));
	int index = 1;
	while( index<=2048)
	{
		void *two_files = osada_get_blocks_relative_since(TABLA_DE_ARCHIVOS,index,1,disco);
		memcpy(file_1,two_files, sizeof(osada_file));
		memcpy(file_2,two_files + sizeof(osada_file), sizeof(osada_file));
		if(verify_file_state(DELETED,file_1))
				{
						count++;
				}
		if(verify_file_state(DELETED,file_2))
			{
						count++;
			}

			free(two_files);
		index++;
	}
	free(a_file_free);
	free(file_1);
	free(file_2);
	return count;
}

int disco_dame_tu_descriptor(char *ruta)
{
	int file_descriptor = open(ruta,O_RDWR);
	return file_descriptor;
}

void* disco_dame_mapping(int size, int file_descriptor)
{
	void *block_pointer = mmap((caddr_t)0,(size_t)size,PROT_READ | PROT_WRITE, MAP_SHARED,file_descriptor,0);
	return block_pointer;
}

int disco_dame_tu_tamanio(char *ruta)
{
	FILE *disco = fopen(ruta, "r");
	fseek(disco, 0L, SEEK_END); // <-- Pongo el puntero del archivo en el ultimo byte
	int tamanio_archivo = ftell(disco);
	fseek(disco, 0L, SEEK_SET); // <-- Pongo el puntero del archivo en el inicio nuevamente
	fclose(disco);
	return tamanio_archivo;
}

osada_header* osada_header_create(void *map)
{
	osada_header *osada_header_new = malloc(sizeof(osada_header));
	memcpy(osada_header_new,map,OSADA_BLOCK_SIZE);
	return osada_header_new;
}

t_bitarray* osada_bitmap_create(t_disco_osada *disco)
{
	char *estructura_bitmap = (char*) osada_get_blocks_relative_since(BITMAP,1,disco->header->bitmap_blocks,disco);
	t_bitarray *bitArray_new = bitarray_create_with_mode(estructura_bitmap,disco->header->bitmap_blocks*OSADA_BLOCK_SIZE,LSB_FIRST);
	return bitArray_new;
}

/*-------------------------------------------------------DICCIONARIO DE ARCHIVOS------------------------------------------*/
int osada_check_exists_in_dictionary(char* path)
{
	return dictionary_has_key(disco->diccionario_de_archivos,path);
}

void osada_change_info_in_dictionary(char* path, osada_block_pointer last_pointer_asig, osada_block_pointer last_block_writed)
{
	char* aux = string_new();
	string_append(&aux,path);
	t_info_file *info = dictionary_remove(disco->diccionario_de_archivos,aux);
	info->last_block_asigned=last_pointer_asig;
	info->last_block_write=last_block_writed;
	dictionary_put(disco->diccionario_de_archivos,aux,info);
}

/*----------------------------------------------IMPACTAR CAMBIOS EN DISCO-------------------------------------------------*/
int osada_hay_espacio_para_archivos()
{
	pthread_mutex_lock(&mutex_operaciones);
	if(disco->cantidad_archivos_libres>0)
	{
		pthread_mutex_unlock(&mutex_operaciones);
		return 1;
	}
	else
	{
		pthread_mutex_unlock(&mutex_operaciones);
		return 0;
	}
}

void osada_aumenta_cantidad_de_archivos()
{
	pthread_mutex_lock(&mutex_operaciones);
	disco->cantidad_archivos_libres = disco->cantidad_archivos_libres +1;
	pthread_mutex_unlock(&mutex_operaciones);
}


void osada_disminui_cantidad_de_archivos()
{
	pthread_mutex_lock(&mutex_operaciones);
	disco->cantidad_archivos_libres = disco->cantidad_archivos_libres -1;
	pthread_mutex_unlock(&mutex_operaciones);
}


void osada_push_middle_block(int campo, int numero_block_relative, int offset, void *bloque, t_disco_osada *disco)
{
	int byte_inicial = calcular_byte_inicial_relative(campo,numero_block_relative,disco->header);
	impactar_en_disco_medio_bloque(byte_inicial + offset,bloque,disco->map);
}

void osada_push_block(int campo, int numero_block_relative, void *bloque,t_disco_osada *disco)
{
	int byte_inicial = calcular_byte_inicial_relative(campo,numero_block_relative,disco->header);
	impactar_en_disco_bloque_completo(byte_inicial,bloque,disco->map);
}

void impactar_en_disco_bloque_completo(int byte_inicial,void *bloque, void *map)
{
	pthread_mutex_lock(&mutex_operaciones);
	int i;
	int byte = 0;
	int byte_final = byte_inicial + OSADA_BLOCK_SIZE-1;
	char *block = (char*) bloque;
	char *mapping = (char*) map;
	for(i=byte_inicial; i<=byte_final;i++)
	{
		mapping[i] = block[byte];
		byte++;
	}
	pthread_mutex_unlock(&mutex_operaciones);
}

void impactar_en_disco_medio_bloque(int byte_inicial,void *bytes, void *map)
{
	pthread_mutex_lock(&mutex_operaciones);
	int i;
	int byte = 0;
	char *block = (char*) bytes;
	int byte_final = byte_inicial + 31;
	char *mapping = (char*) map;
	for(i=byte_inicial; i<=byte_final;i++)
	{
		mapping[i] = block[byte];
		byte++;
	}
	pthread_mutex_unlock(&mutex_operaciones);
}

void impactar_en_disco_n_bloques(int byte_inicial, int cantidad_bloques,void *bloques, void *map)
{
	//pthread_mutex_lock(&mutex_operaciones);
	int i;
	int byte = 0;
	int byte_final = byte_inicial + (OSADA_BLOCK_SIZE)*cantidad_bloques -1;
	char *block = (char*) bloques;
	char *mapping = (char*) map;
	for(i=byte_inicial; i<=byte_final;i++)
	{
			mapping[i] = block[byte];
			byte++;
	}
	//pthread_mutex_unlock(&mutex_operaciones);
}

void impactar_en_disco_tabla_asignaciones(char* new_table)
{
	int tamanio_tabla_asignaciones = calcular_tamanio_tabla_de_asignaciones(disco->header);
	osada_block_pointer byte_inicial_tabla_asignaciones = calcular_byte_inicial_absolut(disco->header->allocations_table_offset);

	pthread_mutex_lock(&mutex_operaciones);
	char* mapping = (char*) disco->map;
	int i;
	for(i=byte_inicial_tabla_asignaciones;i<tamanio_tabla_asignaciones;i++)
	{
		mapping[i]=new_table[i];

	}
	pthread_mutex_unlock(&mutex_operaciones);
}

/*----------------------------------------------OBTENCION DE BLOQUES-------------------------------------------------*/

void* osada_get_blocks_relative_since(int campo, int num_block_init, int num_blocks,t_disco_osada *disco)
{
	switch(campo)
	{
		case(HEADER):
			{
				int byte_inicial = calcular_byte_inicial_relative(HEADER, num_block_init,disco->header);
				return osada_get_block_start_in(byte_inicial,num_blocks,disco->map);
			};break;
		case(BITMAP):
			{
				int byte_inicial = calcular_byte_inicial_relative(BITMAP, num_block_init,disco->header);
				void* valor_Retorno = osada_get_block_start_in(byte_inicial,num_blocks,disco->map);
				return valor_Retorno;
			};break;
		case(TABLA_DE_ARCHIVOS):
			{
				int byte_inicial = calcular_byte_inicial_relative(TABLA_DE_ARCHIVOS, num_block_init,disco->header);
				return osada_get_block_start_in(byte_inicial,num_blocks,disco->map);
			};break;
		case(TABLA_DE_ASIGNACIONES):
			{
				int byte_inicial = calcular_byte_inicial_relative(TABLA_DE_ASIGNACIONES, num_block_init,disco->header);
				return osada_get_block_start_in(byte_inicial,num_blocks,disco->map);
			};break;
		case(BLOQUE_DE_DATOS):
			{
				int byte_inicial = calcular_byte_inicial_relative(BLOQUE_DE_DATOS, num_block_init,disco->header);
				return osada_get_block_start_in(byte_inicial,num_blocks,disco->map);
			};break;
	}
}

void* osada_get_block_start_in(int byte_inicial, int num_blocks, void *map)
{
	void *bloc = malloc(OSADA_BLOCK_SIZE * num_blocks);
	memcpy(bloc,map + byte_inicial ,OSADA_BLOCK_SIZE * num_blocks);
	return bloc;
}

void* osada_get_bytes_start_in(int byte_inicial, int num_bytes_total, void *map)
{
	void *bloc = malloc(num_bytes_total);
	memcpy(bloc,map + byte_inicial ,num_bytes_total);
	return bloc;
}

osada_block_pointer calcular_byte_inicial_relative(int campo, int numero_bloque,osada_header *header)
{

	switch(campo)
	{
		case(BITMAP):
			{
					return (OSADA_BLOCK_SIZE*numero_bloque);


			};break;
		case(TABLA_DE_ARCHIVOS):
			{
				int ultimo_byte_bitmap = OSADA_BLOCK_SIZE * header->bitmap_blocks;
				int primer_byte = ultimo_byte_bitmap + OSADA_BLOCK_SIZE*numero_bloque;
				return(primer_byte);
			};break;
		case(TABLA_DE_ASIGNACIONES):
			{
				if(numero_bloque==1)
				{
					return calcular_byte_inicial_absolut(header->allocations_table_offset);
				}
				else
				{
					int primer_byte = calcular_byte_inicial_absolut(header->allocations_table_offset) + OSADA_BLOCK_SIZE*numero_bloque;
					return primer_byte;
				}
			};break;
		case(BLOQUE_DE_DATOS):
			{
				int tamanio_tabla_asignaciones = calcular_tamanio_tabla_de_asignaciones(header);
				int byte_inicial_bd = calcular_byte_inicial_relative(TABLA_DE_ASIGNACIONES,tamanio_tabla_asignaciones,header);

				int primer_byte_del_bloque_pedido = (byte_inicial_bd) +(OSADA_BLOCK_SIZE*numero_bloque);
				return primer_byte_del_bloque_pedido;
			};break;
	}

}

osada_block_pointer calcular_byte_inicial_absolut(int numero_bloque_absoluto)
{
	if(numero_bloque_absoluto == 0)
	{
		return 0;
	}
	else
	{
		int byte_inicial = numero_bloque_absoluto*64;
		return (byte_inicial);
	}
}

int calcular_tamanio_tabla_de_asignaciones(osada_header *header)
{
	int tamanio_tabla_de_asignaciones = header->fs_blocks - HEADER - TABLA_DE_ARCHIVOS - header->bitmap_blocks - header->data_blocks;
	return tamanio_tabla_de_asignaciones;
}

osada_block_pointer osada_get_start_block_absolut_of(int campo, t_disco_osada *disco)
{
	switch(campo)
	{
			case(HEADER):
				{
					return 0;
				};break;
			case(BITMAP):
				{
					return 1;
				};break;
			case(TABLA_DE_ARCHIVOS):
				{
					return (0 + disco->header->bitmap_blocks +1);
				};break;
			case(TABLA_DE_ASIGNACIONES):
				{
					return disco->header->allocations_table_offset;
				};break;
			case(BLOQUE_DE_DATOS):
				{
					return (disco->header->allocations_table_offset + calcular_tamanio_tabla_de_asignaciones(disco->header));
				};break;
		}
}

osada_file* osada_get_file_for_index(int posicion_en_tabla_de_archivo)
{
	int numero_bloque;
	if(posicion_en_tabla_de_archivo !=0 && es_par(posicion_en_tabla_de_archivo))
	{
		numero_bloque= (posicion_en_tabla_de_archivo/2) +1;
	}
	else
	{
		if(posicion_en_tabla_de_archivo == 0 || posicion_en_tabla_de_archivo ==1)
		{
			numero_bloque = 1;
		}
		else
		{
			numero_bloque = (posicion_en_tabla_de_archivo - 1) /2 +1;
		}
	}

	void* two_files =  osada_get_blocks_relative_since(TABLA_DE_ARCHIVOS,numero_bloque,1,disco);
	osada_file *file_buscado = malloc(sizeof(osada_file));
	if(posicion_en_tabla_de_archivo == 0)
	{
		memcpy(file_buscado,two_files,sizeof(osada_file));
	}
	else
	{
		if(es_par(posicion_en_tabla_de_archivo))
		{
			memcpy(file_buscado,two_files,sizeof(osada_file));
		}
		else
		{
			memcpy(file_buscado,two_files+sizeof(osada_file),sizeof(osada_file));
		}
	}
	free(two_files);
	return file_buscado;
}

void osada_impactar_un_archivo(int posicion_en_tabla_de_archivo, osada_file *file)
{
	int numero_bloque;
	int offset;
	if(posicion_en_tabla_de_archivo !=0 && es_par(posicion_en_tabla_de_archivo))
	{
			numero_bloque= (posicion_en_tabla_de_archivo/2) +1;
			offset=0;
	}
	else
	{
		if(posicion_en_tabla_de_archivo == 0 || posicion_en_tabla_de_archivo ==1)
		{
			numero_bloque = 1;
			if(posicion_en_tabla_de_archivo==0)
			{
				offset=0;
			}
			else
			{
				offset=32;
			}
		}
		else
		{
			numero_bloque = (posicion_en_tabla_de_archivo - 1) /2 +1;
			offset=32;
		}
	}

	osada_push_middle_block(TABLA_DE_ARCHIVOS,numero_bloque,offset,file,disco);

}
/*----------------------------------------------MANIPULACION BITARRAY------------------------------------------------------*/
int osada_b_check_is_bitarray_have_n_blocks_free(int n, t_disco_osada *disco)
{
	return osada_b_check_is_bitarray_have_n_blocks_free_full(n);
}

int osada_b_check_is_bitarray_have_n_blocks_free_full(int n)
{
	pthread_mutex_lock(&mutex_operaciones);
	int resultado = disco->cantidad_bloques_libres - n;
	pthread_mutex_unlock(&mutex_operaciones);
	if(resultado>=0)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

int osada_ocupa_bit_libre_full(t_disco_osada *disco);
int osada_ocupa_bit_libre_de(t_disco_osada *disco)
{
	/*int bloque_incial = osada_get_start_block_absolut_of(BLOQUE_DE_DATOS,disco);
	int i =bloque_incial;
	pthread_mutex_lock(&mutex_operaciones);
	while(bitarray_test_bit(disco->bitmap,i) && i<(bloque_incial + disco->header->data_blocks))
	{
		i++;
	}
	bitarray_set_bit(disco->bitmap,i);
	impactar_en_disco_n_bloques(OSADA_BLOCK_SIZE,disco->header->bitmap_blocks,disco->bitmap->bitarray,disco->map);
	osada_disminui_cantidad_bloques_libres(1);
	pthread_mutex_unlock(&mutex_operaciones);
	return i;*/
	return osada_ocupa_bit_libre_full(disco);
}

int osada_ocupa_bit_libre_full(t_disco_osada *disco)
{
	int bloque_incial = osada_get_start_block_absolut_of(BLOQUE_DE_DATOS,disco);
	int i = primer_bloque_libre_para_asignar;
	int esta_ocupado = 1;
	int valor_final_a_devolver = 0;
	pthread_mutex_lock(&mutex_operaciones);
	while(esta_ocupado && i<(bloque_incial + disco->header->data_blocks))
	{
		esta_ocupado=bitarray_test_bit(disco->bitmap,i);
		i++;
	}
	if(!esta_ocupado)
	{
		bitarray_set_bit(disco->bitmap,i -1);
		primer_bloque_libre_para_asignar = i;
		valor_final_a_devolver = i-1;

	}
	else
	{
		int i2 = bloque_incial;
		while(bitarray_test_bit(disco->bitmap,i2) && i2<(bloque_incial + disco->header->data_blocks))
		{
			i2++;
		}
		bitarray_set_bit(disco->bitmap,i2);
		primer_bloque_libre_para_asignar = i2+1;
		valor_final_a_devolver=i2;
	}
	impactar_en_disco_n_bloques(OSADA_BLOCK_SIZE,disco->header->bitmap_blocks,disco->bitmap->bitarray,disco->map);
	osada_disminui_cantidad_bloques_libres(1);
	pthread_mutex_unlock(&mutex_operaciones);
	return valor_final_a_devolver;
}

int primer_bloque_libre()
{
	int bloque_incial = osada_get_start_block_absolut_of(BLOQUE_DE_DATOS,disco);
	int i =bloque_incial;
	int encontre_bloque_libre = 0;
	while(bitarray_test_bit(disco->bitmap,i) && i<(bloque_incial + disco->header->data_blocks) && !encontre_bloque_libre)
	{
		i++;
	}
	if(encontre_bloque_libre)
	{
		return i--;
	}
	else
	{
		return  FEOF;
	}
}

int calcular_posicion_relativa_en_bloque_de_datos(int posicion_absoluta)
{
	int bloque_incial = osada_get_start_block_absolut_of(BLOQUE_DE_DATOS,disco);
	return (posicion_absoluta- bloque_incial);
}

void osada_desocupa_bit(t_disco_osada *disco, int num_block)
{
	pthread_mutex_lock(&mutex_operaciones);
	bitarray_clean_bit(disco->bitmap,num_block);
	pthread_mutex_unlock(&mutex_operaciones);
}

void osada_desocupa_n_bits(t_list *bloques_a_liberar)
{
	int size = list_size(bloques_a_liberar);
	int i;
	int cantidad_blo_admin = calcular_cantidad_bloques_admin();
	for(i=0; i<size; i++)
	{
		int *bloque=list_get(bloques_a_liberar,i);
		settear_valor_en_tabla_asginaciones(*bloque, FEOF);
		int bit_a_desocupar = cantidad_blo_admin+ *bloque;
		osada_desocupa_bit(disco,bit_a_desocupar);
	}
	osada_aumenta_cantidad_bloques_libres(size);
	impactar_en_disco_n_bloques(OSADA_BLOCK_SIZE,disco->header->bitmap_blocks,disco->bitmap->bitarray,disco->map);
}

void osada_desocupa_archivo_entero(osada_file *file)
{
	osada_block_pointer before_block = file->first_block;
	int cantidad_blo_admin = calcular_cantidad_bloques_admin();
	int hay_mas_para_leer = 1;
	//settear_valor_en_tabla_asginaciones(before_block, FEOF);
	int bit_a_desocupar = cantidad_blo_admin+ before_block;
	osada_desocupa_bit(disco,bit_a_desocupar);
	osada_aumenta_cantidad_bloques_libres(1);

	while(hay_mas_para_leer)
	{
		int block = table_asignaciones[before_block];
		if(block == FEOF)
		{
				hay_mas_para_leer=0;
		}
		else
		{
			//settear_valor_en_tabla_asginaciones(block, FEOF);
			int bit_a_desocupar_2 = cantidad_blo_admin+ block;
			osada_desocupa_bit(disco,bit_a_desocupar_2);
			before_block = block;
			osada_aumenta_cantidad_bloques_libres(1);
		}
	}
	impactar_en_disco_n_bloques(OSADA_BLOCK_SIZE,disco->header->bitmap_blocks,disco->bitmap->bitarray,disco->map);

}

int calcular_cantidad_bloques_admin()
{
	return disco->header->bitmap_blocks + 1 +1024 + calcular_tamanio_tabla_de_asignaciones(disco->header);
}

/*---------------------------------------------TIME---------------------------------------------------------------------*/
void osada_b_actualiza_time(t_file_osada* file)
{
	time_t ahora = time(NULL);
	file->file->lastmod = ahora;
	int offset = calcular_desplazamiento_tabla_de_archivos(file->position_in_block);
	osada_push_middle_block(TABLA_DE_ARCHIVOS,file->block_relative,offset,file->file,disco);
}


/*---------------------------------------------AUXILIARES----------------------------------------------------------------*/
int array_size(char **array)
{
	int cantidad_de_elementos = 0;
	int i = 0;
	while(array[i] !=NULL)
	{
		cantidad_de_elementos++;
		i++;
	}
	return cantidad_de_elementos;
}

void array_free_all(char **array)
{
	int i =0;
	while(array[i] != NULL)
	{
		free(array[i]);
		i++;
	}
	free(array);
}

char* array_last_element(char* path)
{
	char **file_for_file = string_split(path, "/");
	int size = array_size(file_for_file);
	char *nombre = string_new();
	char* ultimo_elemento = file_for_file[size - 1];
	string_append(&nombre,ultimo_elemento);
	array_free_all(file_for_file);
 	return nombre;
}

char* crear_ruta(char* hijo, char* path_padre)
{
	char* path_final = string_new();
	string_append(&path_final,path_padre);
	string_append(&path_final,"/");
	string_append(&path_final,hijo);
	return path_final;
}

char* modelar_nombre_archivo(unsigned char* name_file)
{
	int encontre_feof=0;
	int index =0;
	while(!encontre_feof && index<OSADA_FILENAME_LENGTH)
	{
		if(name_file[index] == '\0')
		{
			encontre_feof=1;
		}
		index++;
	}

	if(encontre_feof)
	{
		char* new_element = string_new();
		string_append(&new_element,(char*)name_file);
		return new_element;
	}
	else
	{
		char* new_element = malloc(sizeof(char)*(OSADA_FILENAME_LENGTH+1));
		int i;
		for(i=0;i<OSADA_FILENAME_LENGTH;i++)
		{
			new_element[i]=name_file[i];
		}
		new_element[OSADA_FILENAME_LENGTH]='\0';

		return new_element;
	}
}

/*---------------------------------------------DESTROYERS----------------------------------------------------------------*/
void t_file_osada_destroy(t_file_osada *file)
{
	free(file->file);
	free(file);
}

void file_listado_eliminate(void* arg)
{
	t_file_listado* file = (t_file_listado*) arg;
	free(file->path_completo);
	free(file->path);
	free(file);
}

void free_list_blocks(void* arg)
{
	int* elem = (int*) arg;
	free(elem);
}


