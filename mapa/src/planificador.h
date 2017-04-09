/*
 * planificador.h
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */

#ifndef MAPA_MAPA_COMMONS_PLANIFICADOR_H_
#define MAPA_MAPA_COMMONS_PLANIFICADOR_H_
#include "so-commons/log.h"
#include <stdio.h>
#include <stdlib.h>
#include "map-commons.h"
#include "planificador-rr.h"
#include "planificadorSRDF.h"
#include "basic-structs.h"
#include "socket/serializacion_mapa_entrenador.h"
#define DESCONECTADO 0


enum
{
	INICIADO,
	NO_INICIADO
}estado_de_encolacion;

enum
{
	COLA_LISTOS,
	COLA_BLOQUEADOS,
	COLA_SIN_OBJETIVOS
}t_cola;


enum
{
	BLOQUEAR_DE_TODOS_MODOS,
	PERMITIR_SI_ES_POSIBLE
}t_trato_captura_pokemon;

t_log *informe_planificador;
extern t_log *informe_cola_listos;
extern t_log *informe_cola_bloqueados;
extern t_log *informe_cola_nuevos;


void planificador_inicia_log();
/* ----------------------------------------STRUCTS----------------------------------------------------------------*/
typedef struct listas_y_colas
{
	t_queue *cola_entrenadores_listos;
	t_queue *cola_entrenadores_bloqueados;
	t_dictionary *lista_entrenadores_finalizados;
	t_list *lista_entrenadores_a_planificar;
}t_listas_y_colas;

typedef struct round_robbin
{
	t_listas_y_colas *listas_y_colas;
	int quantum;
	int retardo;
}t_planificador_rr;

typedef struct srdf
{
	t_listas_y_colas *listas_y_colas;
	t_queue *cola_entrenadores_sin_objetivo;
	t_list *listos_y_ordenados;
	int retardo;
}t_planificador_srdf;

/*--------------------------------------------CREATES---------------------------------------------------------------*/
t_listas_y_colas* listas_y_colas_creense();
t_planificador_rr* planificador_rr_create();
t_planificador_srdf* planificador_srdf_create();
/*--------------------------------------------DESTROYERS---------------------------------------------------------------*/
void planificador_rr_destruite(t_planificador_rr *rr);
void planificador_srdf_destruite(t_planificador_srdf *srdf);

/*-----------------------------------EXECUTE PLANIFICADOR SRDF--------------------------------------------------------*/
void* ejecutar_planificador_srdf(void* arg);

/*---------------------------------------PUSH Y POPS DE COLAS---------------------------------------------------------*/
void planificador_push_entrenador_a_bloqueado(t_entrenador *entrenador);
void planificador_push_entrenador_a_listo(t_entrenador *entrenador);
t_entrenador* planificador_pop_entrenador_bloqueado();
t_entrenador* planificador_pop_entrenador_listo();
t_entrenador* planificador_pop_entrenador_listo_to_srdf();
/*-------------------------------------------FUNCIONES GENERALES--------------------------------------------------------*/
void planificador_dale_coordenadas_a_entrenador(t_entrenador *entrenador);
void planificador_entrenador_se_mueve(t_entrenador *entrenador);
void planificador_entrenador_quiere_capturar_pokemon(t_entrenador *entrenador, int permiso);
void planificador_trata_captura_pokemon(t_entrenador *entrenador);
void planificador_entrega_pokemon_a(t_entrenador *entrenador);
void planificador_bloquea_entrenador(t_entrenador *entrenador);
void planificador_desbloqueame_a(t_entrenador *entrenador);
void cola_bloqueados_quita_entrenador_especifico(t_queue *cola, int id_proceso);
void planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores();
void planificador_desbloquea_entrenador_si_es_posible(int cantidad_bloqueados);

/*---------------------------------------FINALIZADO---------------------------------------------------------*/
void planificador_aborta_entrenador(t_entrenador *entrenador);
void planificador_aborta_entrenador_por_deadlock(t_entrenador* entrenador);
void planificador_finaliza_entrenador(t_entrenador *entrenador);
void planificador_espera_que_entrenador_se_desconecte(t_entrenador *entrenador);
void planificador_extraele_pokemones_a_entrenador(t_entrenador *entrenador);

/*---------------------------------------NUEVO->LISTO---------------------------------------------------------*/
void planificador_inicia_encolacion_nuevos_entrenadores();
void* planificador_encola_nuevos_entrenadores();
void planificador_modela_nuevo_entrenador_y_encolalo(int cola, void *entrenador);
void foreach(int cola,void *lista,void(*funcion_de_lista)(int,void*));

/*---------------------------------------EXECUTE->LISTO---------------------------------------------------------*/
void planificador_volve_a_encolar_a_listo_si_es_necesario(t_entrenador *entrenador);

/*---------------------------------------LOGS---------------------------------------------------------*/
void mostrarTodo(t_queue* cola, int tipo);
char* convertir_a_string(char caracter);
#endif /* MAPA_MAPA_COMMONS_PLANIFICADOR_H_ */
