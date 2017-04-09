/*
 * basic-structs.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef BASIC_STRUCTS_H_
#define BASIC_STRUCTS_H_
#include "so-commons/collections/list.h"
#include "so-commons/config.h"
#include "so-commons/string.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <signal.h>
/* ----------------------------------------STRUCTS----------------------------------------------------------------*/
typedef struct coordenada
{
	int x;
	int y;
}t_ubicacion;

typedef struct mapa
{
	char *nombre;
	int server;
	char* puerto;
	char* ip;
	t_list* objetivos;
	t_list* pokemons_capturados;
	t_config* config;
} t_mapa;


typedef struct entrenador
{
	char* nombre;
	char* ruta_pokedex;
	t_config* configuracion;
	char* directorio_de_bill;
	char* simbolo;
	t_list* hoja_de_viaje;
	t_mapa* mapa_actual;
	int vidas;
	int reintentos;
	int tiempo_total_aventura;
	int tiempo_bloqueado_pokenest;
	int cantidad_de_veces_bloqueado;
	int muertes;
	int cantidad_deadlocks;
	t_ubicacion* ubicacion;
	t_ubicacion* paso_anterior;
	t_ubicacion* pokenest;
}t_entrenador;


/*--------------------------------------------CREATES---------------------------------------------------------------*/
t_ubicacion* ubicacion_create(int x, int y);
t_config* configuracion_metadata_create(char *nombre, char *ruta);
t_config* configuracion_metadata_mapa_create(char *nombre, char *ruta);
t_entrenador* entrenador_create(char* nombre, char* ruta);
t_mapa* mapa_create(char* nombre_mapa, char *ruta_pokedex, t_entrenador *entrenador);

void mapa_destruite(t_mapa *mapa);
void mapa_element_destroyer(void* arg);
void entrenador_destruite(t_entrenador *entrenador);
void hoja_de_viaje_destruite(void* arg);
/*--------------------------------------------OBTENCION DE DATOS---------------------------------------------------------------*/
char* obtener_direccion_directorio_de_bill(char* ruta_pokedex, char* nombre);
t_list* entrenador_hoja_de_viaje(t_config* configuracion);
char** mapas_a_Recorrer(char *mapas_con_corchetes);
t_list* foreach_hoja_de_viaje(char **hoja_de_viaje);
t_list* asociar_objetivos_por_mapa(char *nombre_mapa, t_entrenador *entrenador);
char* entrenador_simbolo(t_config* configuracion);
int entrenador_vidas(t_config* configuracion);

char* obtener_ruta_especifica(char *ruta_inicial, char *directorio_o_nombre_archivo, char *sub_directorio_o_nombre_archivo);

void array_free_all_2(char **array);

#endif /* BASIC_STRUCTS_H_ */
