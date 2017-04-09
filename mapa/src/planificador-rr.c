/*
 * planificador-rr.c
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */
#include "planificador-rr.h"

t_planificador_rr *planificador;
extern sem_t semaforo_entrenadores_listos;
extern int hay_jugadores_online;
extern int algoritmo_cambio;
int semaforo_rr_cambiado_por_deadlock =0;
//extern int encolacion_entrenadores_iniciada;

/*-----------------------------------EXECUTE PLANIFICADOR RR---------------------------------------------------------*/
void* ejecutar_planificador_rr(void* arg)
{
	planificador_inicia_log();
	planificador = planificador_rr_create();
	planificador_rr_organiza_entrenadores();
	free(informe_planificador);
	pthread_exit(NULL);
}

/*--------------------------------------------ROUND ROBBIN----------------------------------------------------------*/
void quamtum_disminuite(int *q)
{
	*q = *q -1;
}

int quamtum_se_termino(int q)
{
	if(q == 0) {return 1;}
	else {return 0;}
}

void planificador_rr_organiza_entrenadores()
{
	while(mapa_decime_si_planificador_es(PLANIFICADOR_RR) && !algoritmo_cambio)
	{
		if(queue_is_empty(planificador->listas_y_colas->cola_entrenadores_listos))
		{
			int se_puede_continuar=0;
			while(!se_puede_continuar)
			{
				hay_jugadores_online =0;
				sem_wait(&semaforo_entrenadores_listos);
				if(semaforo_rr_cambiado_por_deadlock == 1)
				{
					hay_jugadores_online = 1;
					semaforo_rr_cambiado_por_deadlock = 0;
					planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores();
					if(!queue_is_empty(planificador->listas_y_colas->cola_entrenadores_listos))
					{
						se_puede_continuar = 1;
					}
				}
				else
				{
					se_puede_continuar = 1;
				}
			}

		}
		planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores();
		mostrarTodo(planificador->listas_y_colas->cola_entrenadores_bloqueados,COLA_BLOQUEADOS);
		mostrarTodo(planificador->listas_y_colas->cola_entrenadores_listos,COLA_LISTOS);
		t_entrenador *entrenador_listo = planificador_pop_entrenador_listo(planificador);
		int quamtum_restante = mapa->info_algoritmo->quamtum;
		int estado_anterior = entrenador_listo->estado;
		mapa_cambiale_estado_a_entrenador(entrenador_listo, EXECUTE, estado_anterior);
		loggear_turno(entrenador_listo);
		planificador_rr_es_el_turno_de(entrenador_listo, &quamtum_restante);
		log_info(informe_planificador, "Fin de turno");
		planificador_volve_a_encolar_a_listo_si_es_necesario(entrenador_listo);
		planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores();
		//mostrarTodo(planificador->listas_y_colas->cola_entrenadores_bloqueados,COLA_BLOQUEADOS);
		//mostrarTodo(planificador->listas_y_colas->cola_entrenadores_listos,COLA_LISTOS);
	}
	cambiar_algoritmo();
	planificador_rr_destruite(planificador);
}

void loggear_turno(t_entrenador *entrenador)
{
	char* mensaje_a_log_2 = string_new();
	string_append(&mensaje_a_log_2,"TURNO DEL ENTRENADOR: ");
	string_append(&mensaje_a_log_2,entrenador->simbolo_identificador);
	log_info(informe_planificador, mensaje_a_log_2);
	free(mensaje_a_log_2);
}

void loggear_fin_turno(t_entrenador *entrenador)
{
	char* mensaje_a_log_2 = string_new();
	string_append(&mensaje_a_log_2," FIN TURNO DEL ENTRENADOR: ");
	string_append(&mensaje_a_log_2,entrenador->simbolo_identificador);
	log_info(informe_planificador, mensaje_a_log_2);
	free(mensaje_a_log_2);
}

void planificador_rr_es_el_turno_de(t_entrenador *entrenador_listo, int *quamtum)
{
	while(!quamtum_se_termino(*quamtum) && (entrenador_listo->objetivo_cumplido != ABORTADO) )
	{
		usleep(mapa->info_algoritmo->retardo*1000);
		if(mapa_decime_si_entrenador_esta_listo_pero_estaba_bloqueado(entrenador_listo) || entrenador_listo->esperando_pokemon == SI)
		{
			planificador_rr_volve_a_bloquear_a_entrenador_si_es_necesario(entrenador_listo, quamtum);
			if(!quamtum_se_termino(*quamtum))
			{
				planificador_rr_dale_nuevo_turno_a_entrenador(entrenador_listo,quamtum);
			}
		}
		else
		{
			planificador_rr_dale_nuevo_turno_a_entrenador(entrenador_listo,quamtum);
		}
	}

}

void planificador_rr_dale_nuevo_turno_a_entrenador(t_entrenador *entrenador_listo, int *quamtum_restante)
{
	enviar_mensaje_a_entrenador(entrenador_listo,OTORGAR_TURNO,NULL);
	char *mensaje_del_entrenador = escuchar_mensaje_entrenador(entrenador_listo, SOLICITUD_DEL_ENTRENADOR);
	int caso = tratar_respuesta(mensaje_del_entrenador,entrenador_listo);
	free(mensaje_del_entrenador);
	switch(caso)
	{
		case(ENTRENADOR_DESCONECTADO):
			{
				planificador_aborta_entrenador(entrenador_listo);
				*quamtum_restante = 0;
			};break;
		case (ENTRENADOR_ESTA_BUSCANDO_COORDENADAS_POKENEST):
			{
				planificador_dale_coordenadas_a_entrenador(entrenador_listo);
				quamtum_disminuite(quamtum_restante);
				entrenador_listo->tiene_objetivo = SI;
			} break;
		case(ENTRENADOR_QUIERE_MOVERSE):
			{
				planificador_entrenador_se_mueve(entrenador_listo);
				log_info(informe_planificador,"Entrenador se movio");
				quamtum_disminuite(quamtum_restante);
			} break;
		case(ENTRENADOR_QUIERE_CAPTURAR_POKEMON):
			{
				planificador_entrenador_quiere_capturar_pokemon(entrenador_listo, PERMITIR_SI_ES_POSIBLE);
				if(entrenador_listo->estado == BLOQUEADO)
				{
					*quamtum_restante = 0;
					entrenador_listo->esperando_pokemon = SI;
				}
				else { quamtum_disminuite(quamtum_restante); entrenador_listo->tiene_objetivo=NO; }
			} break;
		default:
		{
			planificador_aborta_entrenador(entrenador_listo);
			*quamtum_restante = 0;
		};break;

	}
}

void planificador_rr_volve_a_bloquear_a_entrenador_si_es_necesario(t_entrenador *entrenador, int *quamtum)
{
	if(mapa_decime_si_hay_pokemones_en_pokenest(entrenador->pokenest_objetivo))
	{
		planificador_entrega_pokemon_a(entrenador);
		quamtum_disminuite(quamtum);
		entrenador->esperando_pokemon = NO;
		entrenador->tiene_objetivo = NO;
	}
	else
	{
		mapa_cambiale_estado_a_entrenador(entrenador,BLOQUEADO,EXECUTE);
		planificador_push_entrenador_a_bloqueado(entrenador);
		*quamtum = 0;
	}

}

void planificador_rr_cambia_semaforo_si_es_necesario()
{
	if(hay_jugadores_online == 0)
	{
		semaforo_rr_cambiado_por_deadlock = 1;
		sem_post(&semaforo_entrenadores_listos);
	}
}
