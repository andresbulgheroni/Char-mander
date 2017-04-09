/*
 * deadlock-resolucion.h
 *
 *  Created on: 11/11/2016
 *      Author: utnso
 */

#ifndef DEADLOCK_RESOLUCION_H_
#define DEADLOCK_RESOLUCION_H_

#include "map-commons.h"
#include "basic-structs.h"
#include "comunicacion.h"
#include "pkmn/battle.h"

enum
{
	VICTIMA_ENTRENADOR_1,
	VICTIMA_ENTRENADOR_2,
	VICTIMA_ENTRENADOR_1_POR_DESCONEXION,
	VICTIMA_ENTRENADOR_2_POR_DESCONEXION,
	VICTIMA_EL_MISMO_ENTRENADOR,
	VICTIMA_EL_OTRO_ENTRENADOR,
	VICTIMA_EL_OTRO_ENTRENADOR_POR_DESCONEXION
};

enum
{
	POR_DESCONEXION,
	POR_BATALLA
};
typedef struct
{
	int numero_victima;
	int caso;
	t_pokemon *pokemon_de_la_victima;
	t_entrenador *victima;
}t_perdedor;

void resolver_deadlock_si_es_posible(t_list* involucrados);
void resolver_deadlock_sin_batalla(t_list *involucrados);
void resolver_deadlock_pokemon(t_list* involucrados);

t_list* recuperar_entrenadores_involucrados(t_list* identificadores_involucrados);
void ordenar_entrenadores_segun_tiempo_de_ingreso(t_list* entrenadores);
int comparador_para_batalla(void* elem1, void* elem2);

t_perdedor* efectuar_batalla_pokemon_entre(t_entrenador *entrenador_1, t_entrenador *entrenador_2);
t_perdedor* efectuar_batalla_pokemon_con_el_anterior_perdedor(t_perdedor* anterior_perdedor, t_entrenador *entrenador);
int pokemones_son_iguales(t_pokemon *pok_1, t_pokemon *pok_2);
int retornar_perdedor(t_pokemon *perdedor, t_pokemon *pokemon_entrenador_1, t_pokemon *pokemon_entrenador_2);
void destroy_pokemon(t_pokemon *pokemon);
void cambiar_semaforos_si_es_necesario();

void involucrados_destroyer(void* arg);
#endif /* DEADLOCK_RESOLUCION_H_ */
