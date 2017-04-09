/*
 * planificadorSRDF.h
 *
 *  Created on: 30/9/2016
 *      Author: utnso
 */

#ifndef PLANIFICADORSRDF_H_
#define PLANIFICADORSRDF_H_

#include "map-commons.h"
#include "comunicacion.h"
#include "basic-structs.h"
#include "planificador.h"
#include "unistd.h"



void* ejecutar_planificador_srdf(void* arg);
void planificador_srdf_organiza_entrenadores();
void planificador_srdf_es_el_turno_de(t_entrenador *entrenador);
void planificador_srdf_dale_nuevo_turno_hasta_que_se_bloquee(t_entrenador *entrenador);
void planificador_srdf_dale_pokemon_si_es_posible(t_entrenador *entrenador);
void planificador_srdf_cambia_semaforo_si_es_necesario();


#endif /* PLANIFICADORSRDF_H_ */
