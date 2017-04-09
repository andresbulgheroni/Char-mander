/*
 * planificador-rr.h
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */

#ifndef PLANIFICADOR_RR_H_
#define PLANIFICADOR_RR_H_

#include "map-commons.h"
#include "comunicacion.h"
#include "basic-structs.h"
#include "unistd.h"

/*-----------------------------------EXECUTE PLANIFICADOR RR---------------------------------------------------------*/
void* ejecutar_planificador_rr(void* arg);

/*--------------------------------------------ROUND ROBBIN----------------------------------------------------------*/
void planificador_rr_organiza_entrenadores();
void planificador_rr_es_el_turno_de(t_entrenador *entrenador_listo, int *quamtum);
void planificador_rr_dale_nuevo_turno_a_entrenador(t_entrenador *entrenador_listo, int *quamtum_restante);
void planificador_rr_volve_a_bloquear_a_entrenador_si_es_necesario(t_entrenador *entrenador, int *quamtum);;
int quamtum_se_termino(int q);
void quamtum_disminuite(int *q);
void planificador_rr_cambia_semaforo_si_es_necesario();
void loggear_turno(t_entrenador *entrenador);
void loggear_fin_turno(t_entrenador *entrenador);
#endif /* PLANIFICADOR_RR_H_ */
