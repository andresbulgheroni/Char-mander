/*
 * planificadorSRDF.c
 *
 *  Created on: 30/9/2016
 *      Author: utnso
 */
#include "planificadorSRDF.h"

t_planificador_srdf *planificador;
pthread_mutex_t mutex_manipular_listos_y_odenados = PTHREAD_MUTEX_INITIALIZER;
extern sem_t semaforo_esperar_por_entrenador_listo;
extern pthread_mutex_t mutex_manipular_cola_listos;
extern int hay_jugadores;
int semaforo_srdf_cambiado_por_deadlock =0;
extern t_log *informe_planificador;
extern int algoritmo_cambio;
t_entrenador *ultimo_ejecutado;

t_list* cola_listos_a_lista(t_queue *cola_listos)
{
	int size = queue_size(cola_listos);
	int i;
	t_list *list_aux = list_create();
	for(i=0;i<size;i++)
	{
		t_entrenador *entrenador =  queue_pop(cola_listos);
		list_add(list_aux,entrenador);
	}
	return list_aux;
}

int comparador_2(void *arg1, void *arg2)
{
	t_entrenador *entrenador_1= (t_entrenador*)arg1;
	t_entrenador *entrenador_2= (t_entrenador*)arg2;

	if(entrenador_1->tiene_objetivo == NO  && entrenador_2->tiene_objetivo ==SI) {return 1;}
	else
	{
		if(entrenador_1->tiene_objetivo == SI  && entrenador_2->tiene_objetivo ==NO){return 0;}
		else
		{
			if(entrenador_1->tiene_objetivo == NO  && entrenador_2->tiene_objetivo ==NO){return 1;}
			else
			{
				if(entrenador_1->distancia_hasta_objetivo <= entrenador_2->distancia_hasta_objetivo) {return 1;}
				else {return 0;}
			}

		}
	}
}

void planificador_srdf_reordena_entrenadores_si_es_necesario_2()
{
	if(queue_size(planificador->listas_y_colas->cola_entrenadores_listos)>1)
	{
		pthread_mutex_lock(&mutex_manipular_cola_listos);

		list_sort(planificador->listas_y_colas->cola_entrenadores_listos->elements,comparador_2);
		pthread_mutex_unlock(&mutex_manipular_cola_listos);
	}

}

int comparador_3(void *arg1, void *arg2)
{
	t_entrenador *entrenador_1= (t_entrenador*)arg1;
	t_entrenador *entrenador_2= (t_entrenador*)arg2;

	if(ultimo_ejecutado != NULL)
	{
		if(entrenador_1->numero_de_ingreso <= entrenador_2->numero_de_ingreso)
		{
			if(!string_equals_ignore_case(entrenador_1->simbolo_identificador,ultimo_ejecutado->simbolo_identificador)){return 1;}
			else return 0;
		}
		else
		{
			if(string_equals_ignore_case(entrenador_2->simbolo_identificador,ultimo_ejecutado->simbolo_identificador)){return 1;}
			else return 0;
		}
	}
	else
	{
		if(entrenador_1->numero_de_ingreso <= entrenador_2->numero_de_ingreso) {return 1;}
		else {return 0;}
	}


}

void planificador_srdf_ordena_todo_como_estaba()
{
	if(queue_size(planificador->listas_y_colas->cola_entrenadores_listos)>1)
	{
		pthread_mutex_lock(&mutex_manipular_cola_listos);
		list_sort(planificador->listas_y_colas->cola_entrenadores_listos->elements,comparador_3);
		pthread_mutex_unlock(&mutex_manipular_cola_listos);
	}
}


/*-----------------------------------EXECUTE PLANIFICADOR SRDF--------------------------------------------------------*/
void* ejecutar_planificador_srdf(void* arg)
{
	planificador_inicia_log();
	planificador = planificador_srdf_create();
	ultimo_ejecutado = NULL;
	planificador_srdf_organiza_entrenadores();
	planificador_srdf_destruite(planificador);
	free(informe_planificador);
	pthread_exit(NULL);
}

void planificador_srdf_organiza_entrenadores()
{
	while(mapa_decime_si_planificador_es(PLANIFICADOR_SRDF) && !algoritmo_cambio)
	{
		if(queue_is_empty(mapa->entrenadores->cola_entrenadores_listos))
		{
			int se_puede_continuar = 0;
			while(!se_puede_continuar)
			{
				hay_jugadores=0;
				sem_wait(&semaforo_esperar_por_entrenador_listo);
				if(semaforo_srdf_cambiado_por_deadlock==1)
				{
					hay_jugadores = 1;
					semaforo_srdf_cambiado_por_deadlock=0;
					planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores();
					if(!queue_is_empty(planificador->listas_y_colas->cola_entrenadores_listos))
					{
						se_puede_continuar = 1;
					}
				}
				else
				{
					planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores();
					if(!queue_is_empty(planificador->listas_y_colas->cola_entrenadores_listos))
					{
						se_puede_continuar = 1;
					}

				}
			}

		}

		planificador_srdf_reordena_entrenadores_si_es_necesario_2();
		mostrarTodo(planificador->listas_y_colas->cola_entrenadores_bloqueados,COLA_BLOQUEADOS);
		mostrarTodo(planificador->listas_y_colas->cola_entrenadores_listos,COLA_LISTOS);

		planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores();
		t_entrenador *entrenador_listo = planificador_pop_entrenador_listo(planificador);
		int estado_anterior = entrenador_listo->estado;
		mapa_cambiale_estado_a_entrenador(entrenador_listo, EXECUTE, estado_anterior);
		loggear_turno(entrenador_listo);
		planificador_srdf_es_el_turno_de(entrenador_listo);
		log_info(informe_planificador, "Fin de turno");
		planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores();

	}

	planificador_srdf_ordena_todo_como_estaba();
	cambiar_algoritmo();
}

void planificador_srdf_es_el_turno_de(t_entrenador *entrenador)
{
	if(mapa_decime_si_entrenador_esta_ejecutando_pero_estaba_bloqueado(entrenador) || entrenador->esperando_pokemon == SI)
	{
		planificador_srdf_dale_pokemon_si_es_posible(entrenador);
	}
	else
	{
		planificador_srdf_dale_nuevo_turno_hasta_que_se_bloquee(entrenador);
	}
}

void planificador_srdf_dale_pokemon_si_es_posible(t_entrenador *entrenador)
{
	/*if(mapa_decime_si_hay_pokemones_en_pokenest(entrenador->pokenest_objetivo))
	{
		planificador_entrega_pokemon_a(entrenador);
		entrenador->esperando_pokemon = NO;
		planificador_push_entrenador_en_cola_sin_objetivos(entrenador);

	}
	else
	{
		mapa_cambiale_estado_a_entrenador(entrenador,BLOQUEADO,EXECUTE);
		planificador_push_entrenador_a_bloqueado(entrenador);
	}*/
	if(mapa_decime_si_hay_pokemones_en_pokenest(entrenador->pokenest_objetivo))
	{
		planificador_entrega_pokemon_a(entrenador);
		entrenador->esperando_pokemon = NO;
		entrenador->tiene_objetivo = NO;
		planificador_push_entrenador_a_listo(entrenador);
	}
	else
	{
		mapa_cambiale_estado_a_entrenador(entrenador,BLOQUEADO,EXECUTE);
		planificador_push_entrenador_a_bloqueado(entrenador);
	}
}

void planificador_srdf_dale_nuevo_turno_hasta_que_se_bloquee(t_entrenador *entrenador)
{
	int fin_rafaga = NO;
	while(!fin_rafaga && !mapa_decime_si_entrenador_esta_abortado(entrenador))
	{
		usleep(mapa->info_algoritmo->retardo*1000);
		enviar_mensaje_a_entrenador(entrenador,OTORGAR_TURNO,NULL);
		char *mensaje_del_entrenador = escuchar_mensaje_entrenador(entrenador, SOLICITUD_DEL_ENTRENADOR);
		int caso = tratar_respuesta(mensaje_del_entrenador,entrenador);
		free(mensaje_del_entrenador);

		switch(caso)
		{
			case(ENTRENADOR_ESTA_BUSCANDO_COORDENADAS_POKENEST):
			{
				planificador_dale_coordenadas_a_entrenador(entrenador);
				entrenador->tiene_objetivo = SI;
				fin_rafaga = SI;
				planificador_push_entrenador_a_listo(entrenador);
				ultimo_ejecutado = entrenador;
			};break;
			case(ENTRENADOR_QUIERE_MOVERSE): planificador_entrenador_se_mueve(entrenador);break;
			case(ENTRENADOR_QUIERE_CAPTURAR_POKEMON):
			{
				planificador_entrenador_quiere_capturar_pokemon(entrenador, PERMITIR_SI_ES_POSIBLE);
				if(mapa_decime_si_entrenador_esta_bloqueado(entrenador))
					{
						entrenador->esperando_pokemon =SI ;
						ultimo_ejecutado = entrenador;
					}
				else
					{
						entrenador->tiene_objetivo = NO;
						planificador_push_entrenador_a_listo(entrenador);
						ultimo_ejecutado = entrenador;
					}
					fin_rafaga = SI;
			}break;
			case(ENTRENADOR_DESCONECTADO):{ planificador_aborta_entrenador(entrenador); ultimo_ejecutado = NULL; };break;
		}
	}

}

void planificador_srdf_cambia_semaforo_si_es_necesario()
{
	if(hay_jugadores==0)
	{
		semaforo_srdf_cambiado_por_deadlock = 1;
		sem_post(&semaforo_esperar_por_entrenador_listo);
	}
}

