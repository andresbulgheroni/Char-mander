/*
 * basic-structs.c
 *
 *  Created on: 12/9/2016
 *      Author: utnso
 */
#include "basic-structs.h"

int numero_entrada_al_mapa = 0;

/*--------------------------------------------CREATES---------------------------------------------------------------*/
t_entrenador* entrenador_create(int id_proceso, int socket_entrenador)
{
	t_entrenador *new_entrenador = malloc(sizeof(t_entrenador));
	new_entrenador->id_proceso = id_proceso;
	new_entrenador->socket_entrenador =socket_entrenador;
	new_entrenador->estado = NONE;
	new_entrenador->estado_anterior = NONE;
	new_entrenador->esperando_pokemon=NONE;
	new_entrenador->objetivo_cumplido = 0;
	new_entrenador->tiene_objetivo = 0;
	new_entrenador->pokemones_capturados = list_create();
	new_entrenador->posicion_actual = posicion_create(0,0);
	new_entrenador->numero_de_ingreso = numero_entrada_al_mapa;
	new_entrenador->debe_liberar_solicitud = NO;
	new_entrenador->esta_en_deadlock = NO;
	new_entrenador->pokenest_objetivo = NULL;
	numero_entrada_al_mapa++;
	return new_entrenador;
}

t_pokeNest* pokenest_create(char* nombre,char *ruta)
{
	t_pokeNest *new_pokenest = malloc(sizeof(t_pokeNest));
	new_pokenest->ruta_en_pokeDex = string_new();
	string_append(&new_pokenest->ruta_en_pokeDex,ruta);

	new_pokenest->nombre = string_new();
	string_append(&new_pokenest->nombre,nombre);

	char* ruta_config = obtener_ruta_especifica(ruta,"metadata", NULL);
	new_pokenest->configuracion = config_create(ruta_config);
	free(ruta_config);

	new_pokenest->tipo = obtener_info_pokenest_tipo(new_pokenest->configuracion);
	new_pokenest->posicion = obtener_info_pokenest_posicion(new_pokenest->configuracion);
	new_pokenest->identificador = obtener_info_pokenest_id(new_pokenest->configuracion);
	new_pokenest->pokemones = obtener_info_pokenest_pokemones(nombre, ruta, new_pokenest->identificador);
	new_pokenest->cantidad_pokemones_disponibles = queue_size(new_pokenest->pokemones);

	config_destroy(new_pokenest->configuracion);

	return new_pokenest;
}

t_posicion* posicion_create(int x, int y)
{
	t_posicion *new_posicion = malloc(sizeof(t_posicion));
	new_posicion->x = x;
	new_posicion->y = y;
	return new_posicion;
}

t_mapa* mapa_create(char *nombre, char *rutaPokedex)
{
	t_mapa *new_mapa = malloc(sizeof(t_mapa));
	new_mapa->nombre = string_new();
	string_append(&new_mapa->nombre,nombre);

	new_mapa->ruta_pokedex = string_new();
	string_append(&new_mapa->ruta_pokedex,rutaPokedex);

	new_mapa->configuracion = configuracion_metadata_create(nombre, rutaPokedex);
	new_mapa->entrenadores = controllers_create();
	new_mapa->info_socket = obtener_info_mapa_socket(new_mapa->configuracion);
	new_mapa->info_algoritmo = obtener_info_mapa_algoritmo(new_mapa->configuracion);
	new_mapa->batalla = obtener_info_mapa_batalla(new_mapa->configuracion);
	new_mapa->tiempo_chequeo_deadlock = obtener_info_mapa_tiempo_deadlock(new_mapa->configuracion);
	new_mapa->pokeNests = obtener_info_mapa_pokenest(nombre,rutaPokedex);
	new_mapa->diccionario_de_entrenadores = dictionary_create();
	return new_mapa;

}

t_controllers* controllers_create()
{
	t_controllers *new_controllers = malloc(sizeof(t_controllers));
	new_controllers->cola_entrenadores_listos = queue_create();
	new_controllers->cola_entrenadores_bloqueados = queue_create();
	new_controllers->lista_entrenadores_finalizados = dictionary_create();
	new_controllers->lista_entrenadores_a_planificar = list_create();
	return new_controllers;
}

t_info_socket* info_socket_create(int puerto, char *ip)
{
	t_info_socket *new_info_socket = malloc(sizeof(t_info_socket));
	new_info_socket->ip =string_new();
	string_append(&new_info_socket->ip,ip);
	new_info_socket->puerto=puerto;
	return new_info_socket;
}

t_info_algoritmo* info_algoritmo_create(char *algoritmo, int quamtum, int retardo)
{
	t_info_algoritmo *new_info_algoritmo = malloc(sizeof(t_info_algoritmo));
	new_info_algoritmo->algoritmo = string_new();
	string_append(&new_info_algoritmo->algoritmo,algoritmo);
	new_info_algoritmo->quamtum = quamtum;
	new_info_algoritmo->retardo = retardo;
	return new_info_algoritmo;
}

t_config* configuracion_metadata_create(char *nombre, char *ruta)
{
	char* ruta_final = obtener_ruta_especifica(ruta, "Mapas", nombre);
	char* ruta_final_full = obtener_ruta_especifica(ruta_final, "metadata", NULL);
	free(ruta_final);
	t_config *config_new = config_create(ruta_final_full);
	free(ruta_final_full);
	return config_new;
}

/*-----------------------------------------------------DESTROYERS----------------------------------------------------*/
void destroy_info_algoritmo(t_info_algoritmo *info)
{
	free(info->algoritmo);
	free(info);
}
/*---------------------------- FUNCIONES PARA OBTENER DATOS BASICOS DE UN MAPA--------------------------------------*/

t_info_socket* obtener_info_mapa_socket(t_config *configuracion)
{
	int port=0;
	char* ip_del_mapa = config_get_string_value(configuracion, "IP");
	if(ip_del_mapa==NULL)
	{
		printf("Se necesita configurar el PUERTO e IP del mapa para iniciar\n");
		printf("Revisa el archivo metadata!\n");
		exit(1);
	}
	else
	{
		port = config_get_int_value(configuracion, "Puerto");
		t_info_socket *info_socket = info_socket_create(port, ip_del_mapa);
		return info_socket;
	}

}

t_info_algoritmo* obtener_info_mapa_algoritmo(t_config *configuracion)
{
	t_info_algoritmo *info_algoritmo = info_algoritmo_create(config_get_string_value(configuracion, "algoritmo"), config_get_int_value(configuracion, "quantum"), config_get_int_value(configuracion, "retardo"));
	return info_algoritmo;
}

int obtener_info_mapa_batalla(t_config *configuracion)
{
	int batalla = config_get_int_value(configuracion, "Batalla");
	return batalla;
}

int obtener_info_mapa_tiempo_deadlock(t_config *configuracion)
{
	int tiempo = config_get_int_value(configuracion,"TiempoChequeoDeadlock");
	return tiempo;
}

t_dictionary* obtener_info_mapa_pokenest(char *nombreMapa, char *rutaPokedex)
{
	t_dictionary *new_dictionary = dictionary_create();

	char *ruta_final = obtener_ruta_especifica(rutaPokedex, "Mapas", nombreMapa);
	char* ruta_final_full= obtener_ruta_especifica(ruta_final, "PokeNests", NULL);
	free(ruta_final);

	t_list *lista_directorios = nombre_de_archivos_del_directorio(ruta_final_full);

	char **vector = malloc((list_size(lista_directorios) +1)*4);

	foreach_pokenest_modelate(lista_directorios,new_dictionary, ruta_final_full, vector);

	char* menos_uno = string_new();
	string_append(&menos_uno,"-1");
	vector[list_size(lista_directorios)] = menos_uno;

	vector_auxiliar_identificadores_pokenest = vector;

	list_destroy_and_destroy_elements(lista_directorios,free_names_dir);

	free(ruta_final_full);

	return new_dictionary;
}

void foreach_pokenest_modelate(void *lista_origen,void *lista_destino, void *ruta,char **vector_aux)
{
	t_list *lista_pokemones_a_modelar = (t_list*)lista_origen;
	t_dictionary *lista_pokemons_a_devolver = (t_dictionary*)lista_destino;

	int tamanio = list_size(lista_pokemones_a_modelar);
	int i;
	char *ruta_final = (char*) ruta;
	for(i=0; i<tamanio;i++)
	{
		char *elemento =list_get(lista_pokemones_a_modelar, i);
		char* ruta_final_full = obtener_ruta_especifica(ruta_final, elemento, NULL);
		t_pokeNest *pokenest = pokenest_create(elemento,ruta_final_full);
		free(ruta_final_full);

		char* id_aux = string_new();
		string_append(&id_aux,pokenest->identificador);

		dictionary_put(lista_pokemons_a_devolver, id_aux, pokenest);
		vector_aux[i] = id_aux;
	}

}


/*---------------------------- FUNCIONES PARA OBTENER DATOS BASICOS DE UN POKENEST--------------------------------------*/
char* obtener_info_pokenest_tipo(t_config *configuracion)
{
	char* aux = string_new();
	string_append(&aux,config_get_string_value(configuracion, "Tipo"));
	return aux;
}

t_posicion* obtener_info_pokenest_posicion(t_config *configuracion)
{
	char *string_de_coordenada = config_get_string_value(configuracion, "Posicion");
	char **array_de_coordenada = string_split(string_de_coordenada, ";");
	int x =  atoi(array_de_coordenada[0]);
	int y =  atoi(array_de_coordenada[1]);
	t_posicion *new_posicion = posicion_create(x,y);
	array_free_all(array_de_coordenada);
	return new_posicion;
}

char* obtener_info_pokenest_id(t_config *configuracion)
{
	char* aux = string_new();
	string_append(&aux,config_get_string_value(configuracion, "Identificador"));
	return aux;
}

t_queue* obtener_info_pokenest_pokemones(char *nombrePokenest, char *ruta_final, char *identificador)
{
	t_queue *new_cola_pokemones = queue_create();
	t_list *lista_directorios = nombre_de_pokemones(ruta_final);
	foreach_pokenest(lista_directorios, new_cola_pokemones, ruta_final, nombrePokenest);
	list_destroy_and_destroy_elements(lista_directorios,free_names_dir);
	return new_cola_pokemones;
}

void free_names_dir(void* arg)
{
	char* name = (char*) arg;
	free(name);
}

void foreach_pokenest(void *lista_origen,void *lista_destino, void *ruta, void *nombre_pokenest)
{
	t_list *lista_pokemones_a_modelar = (t_list*)lista_origen;
	t_queue *cola_pokemons_a_devolver = (t_queue*)lista_destino;

	int tamanio = list_size(lista_pokemones_a_modelar);
	int i;
	for(i=0; i<tamanio;i++)
	{
		char *ruta_final =string_new();
		string_append(&ruta_final, (char*) ruta);
		char *elemento =list_get(lista_pokemones_a_modelar, i);
		char* ruta_final_full = obtener_ruta_especifica(ruta_final, elemento, NULL);
		free(ruta_final);
		queue_push(cola_pokemons_a_devolver, ruta_final_full);
	}

}

char* obtener_id_ponekest(char *ruta_pokemon_determinado)
{
	char **por_separado_aux = string_split(ruta_pokemon_determinado,"/");
	int cantidad_elementos = array_size(por_separado_aux);

	array_free_all(por_separado_aux);

	char* elemento_buscado = array_get_element(ruta_pokemon_determinado,cantidad_elementos-2);
	char* aux = string_substring_until(elemento_buscado,1);
	free(elemento_buscado);
	string_to_upper(aux);
	return aux;
}
/*---------------------------------FUNCIONES PARA MANEJO DE RUTAS----------------------------------------------------------*/

char* obtener_ruta_especifica(char *ruta_inicial, char *directorio_o_nombre_archivo, char *sub_directorio_o_nombre_archivo)
{
	char* ruta = string_new();
	string_append(&ruta,ruta_inicial);
	string_append(&ruta, "/");
	string_append(&ruta, directorio_o_nombre_archivo);
	if(sub_directorio_o_nombre_archivo != NULL)
	{	string_append(&ruta, "/");
		string_append(&ruta,sub_directorio_o_nombre_archivo);
		//string_trim_left(&ruta);
		return ruta;
	}
	else
		{
			//string_trim_left(&ruta);
			return ruta;
		}
}

t_list* nombre_de_archivos_del_directorio(char *ruta)
{

	/* Variables */
		DIR *dirp;
		struct dirent *direntp;

	/* Abrimos el directorio */
		dirp = opendir(ruta);
		if (dirp == NULL){
			printf("Error: No se puede abrir el directorio\n");
			exit(1);
		}
		t_list *lista = list_create();
		while ((direntp = readdir(dirp)) != NULL) {
			 if(!string_equals_ignore_case(direntp->d_name, ".")){
				  if(!string_equals_ignore_case(direntp->d_name, "..")){
					  if(!string_equals_ignore_case(direntp->d_name, "metadata"))
					  {
						  char* path_aux = string_new();
						  string_append(&path_aux,direntp->d_name);
						  list_add(lista, path_aux);
						 //list_add(lista, direntp->d_name);
					  }
				  }
			 }
		}
		 closedir(dirp);
		 	 return lista;
		 	 /* Cerramos el directorio */

	}

t_list* nombre_de_pokemones(char *ruta)
{

	/* Variables */
		DIR *dirp;
		struct dirent *direntp;

	/* Abrimos el directorio */
		dirp = opendir(ruta);
		if (dirp == NULL){
			printf("Error: No se puede abrir el directorio\n");
			exit(1);
		}
		t_list *lista = list_create();
		while ((direntp = readdir(dirp)) != NULL) {
			 if(!string_equals_ignore_case(direntp->d_name, ".")){
				  if(!string_equals_ignore_case(direntp->d_name, "..")){
					  if(!string_equals_ignore_case(direntp->d_name, "metadata") && string_ends_with(direntp->d_name,".dat"))
					  {
						  char* path_aux = string_new();
						  string_append(&path_aux,direntp->d_name);
						  list_add(lista, path_aux);
					  }
				  }
			 }
		}
		 closedir(dirp);
		 	 return lista;

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
	string_append(&nombre,file_for_file[size - 1]);
	array_free_all(file_for_file);
 	return nombre;
}

char* array_get_element(char* path, int element)
{
	char **file_for_file = string_split(path, "/");
	char *nombre = string_new();
	string_append(&nombre,file_for_file[element]);
	array_free_all(file_for_file);
	 return nombre;
}
