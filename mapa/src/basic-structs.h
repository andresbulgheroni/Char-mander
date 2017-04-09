/*
 * basic-structs.h
 *
 *  Created on: 12/9/2016
 *      Author: utnso
 */

#ifndef MAPA_MAPA_COMMONS_BASIC_STRUCTS_H_
#define MAPA_MAPA_COMMONS_BASIC_STRUCTS_H_

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <dirent.h>
#include <ctype.h>
#include "so-commons/collections/dictionary.h"
#include "so-commons/collections/list.h"
#include "so-commons/collections/queue.h"
#include "so-commons/config.h"
#include "so-commons/string.h"
#include "so-commons/txt.h"
#include "semaphore.h"
#include <signal.h>

#define SI 1
#define NO 0

enum
{
	CUMPLIDO = 1,
	NO_CUMPLIDO= 0,
	ABORTADO = -1
}estado_objetivo;

enum
{
	EXECUTE = 2,
	LISTO = 1,
	BLOQUEADO = 0,
	SIN_OBJETIVO = 3,
	NONE = 4,
	MUERTO = -1
}estados_del_entrenador;

char** vector_auxiliar_identificadores_pokenest;

void free_names_dir(void* arg);
/* ----------------------------------------STRUCTS----------------------------------------------------------------*/
typedef struct posicion
{
	int x;
	int y;
}t_posicion;

typedef struct entrenador_nuevo
{
	int id_proceso;
	int socket_entrenador;
	char *simbolo_identificador;
	sem_t *semaforo_finalizacion;
}t_entrenador_nuevo;

typedef struct pokeNest
{
	char *tipo;
	char *nombre;
	char *ruta_en_pokeDex;
	t_posicion *posicion;
	t_config *configuracion;
	char *identificador;
	t_queue *pokemones;
	int cantidad_pokemones_disponibles;
}t_pokeNest;

typedef struct entrenador
{
	int id_proceso;
	int socket_entrenador;
	t_posicion *posicion_actual;
	char *pokenest_objetivo;
	char *simbolo_identificador;
	int tiene_objetivo;
	int distancia_hasta_objetivo;
	int objetivo_cumplido;
	int estado;
	int estado_anterior;
	int esperando_pokemon;
	int numero_de_ingreso;
	int debe_liberar_solicitud;
	t_list *pokemones_capturados;
	//sem_t *semaforo_finalizacon;
	int esta_en_deadlock;
}t_entrenador;

typedef struct controllers
{
	t_queue *cola_entrenadores_listos;
	t_queue *cola_entrenadores_bloqueados;
	t_dictionary *lista_entrenadores_finalizados;
	t_list *lista_entrenadores_a_planificar;
}t_controllers;

typedef struct info_socket
{
	char *ip;
	int puerto;
}t_info_socket;

typedef struct info_algoritmo
{
	char *algoritmo;
	int quamtum;
	int retardo;
}t_info_algoritmo;

typedef struct mapa
{
	char *nombre;
	char *ruta_pokedex;
	t_config *configuracion;
	t_controllers *entrenadores;
	int tiempo_chequeo_deadlock;
	int batalla;
	t_info_algoritmo *info_algoritmo;
	t_info_socket *info_socket;
	t_dictionary *pokeNests;
	t_dictionary *diccionario_de_entrenadores;
	t_list *items_para_mostrar_en_pantalla;
}t_mapa;

/*--------------------------------------------CREATES---------------------------------------------------------------*/
t_mapa* mapa_create(char *nombre, char *rutaPokedex);
t_controllers* controllers_create();
t_info_algoritmo* info_algoritmo_create(char *algoritmo, int quamtum, int retardo);
t_info_socket* info_socket_create(int puerto, char *ip);
t_entrenador* entrenador_create(int id_proceso, int socket_entrenador);
t_config* configuracion_metadata_create(char *nombre, char *ruta);

t_pokeNest* pokenest_create(char *nombre, char *ruta);
t_posicion* posicion_create(int x, int y);

/*-----------------------------------------------------DESTROYERS----------------------------------------------------*/
void destroy_info_algoritmo(t_info_algoritmo *info);
/*--------------------------------FUNCIONES PARA OBTENER DATOS BASICOS DE UN MAPA-----------------------------------*/

t_info_socket* obtener_info_mapa_socket(t_config *configuracion);
t_info_algoritmo* obtener_info_mapa_algoritmo(t_config *configuracion);
int obtener_info_mapa_batalla(t_config *configuracion);
int obtener_info_mapa_tiempo_deadlock(t_config *configuracion);

t_dictionary* obtener_info_mapa_pokenest(char *nombreMapa, char *rutaPokedex);
void foreach_pokenest_modelate(void *lista_origen,void *lista_destino, void *ruta, char** vector_aux);
char* obtener_ruta_especifica(char *ruta_inicial, char *directorio_o_nombre_archivo, char *sub_directorio_o_nombre_archivo);
t_list* nombre_de_archivos_del_directorio(char *ruta);
/* ---------------------------------FUNCIONES PARA OBTENER DATOS DE UN POKENEST------------------------------------------*/
char* obtener_info_pokenest_tipo(t_config *configuracion);
t_posicion* obtener_info_pokenest_posicion(t_config *configuracion);
char* obtener_info_pokenest_id(t_config *configuracion);
t_queue* obtener_info_pokenest_pokemones(char *nombrePokenest, char *ruta, char *identificador);
void foreach_pokenest(void *lista_origen,void *lista_destino, void *ruta, void *identificador);
char* obtener_id_ponekest(char *ruta_pokemon_determinado);

void free_string_array(char **path);

/*---------------------------------------------AUXILIARES----------------------------------------------------------------*/
int array_size(char **array);
void array_free_all(char **array);
char* array_last_element(char* path);
char* array_get_element(char* path, int element);
t_list* nombre_de_pokemones(char *ruta);
#endif /* MAPA_MAPA_COMMONS_BASIC_STRUCTS_H_ */
