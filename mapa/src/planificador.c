/*
 * planificador.c
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */
#include "planificador.h"

extern t_mapa *mapa;
extern sem_t semaforo_entrenadores_listos;
extern pthread_mutex_t mutex_manipular_cola_listos;
extern pthread_mutex_t mutex_manipular_cola_bloqueados;
extern pthread_mutex_t mutex_manipular_cola_nuevos;
extern int hay_jugadores_online;
extern int hay_jugadores;
int encolacion_entrenadores_iniciada;
extern sem_t semaforo_esperar_por_entrenador_listo;


/*--------------------------------------------CREATES---------------------------------------------------------------*/
t_listas_y_colas* listas_y_colas_creense()
{
	t_listas_y_colas *planificador_new = malloc(sizeof(t_listas_y_colas));
	planificador_new->cola_entrenadores_bloqueados=mapa->entrenadores->cola_entrenadores_bloqueados;
	planificador_new->cola_entrenadores_listos=mapa->entrenadores->cola_entrenadores_listos;
	planificador_new->lista_entrenadores_a_planificar=mapa->entrenadores->lista_entrenadores_a_planificar;
	planificador_new->lista_entrenadores_finalizados=mapa->entrenadores->lista_entrenadores_finalizados;
	return planificador_new;
}

t_planificador_rr* planificador_rr_create()
{
	t_planificador_rr *planif_new = malloc(sizeof(t_planificador_rr));
	planif_new->listas_y_colas = listas_y_colas_creense();
	planif_new->quantum = mapa->info_algoritmo->quamtum;
	planif_new->retardo = mapa->info_algoritmo->retardo;
	return planif_new;
}

t_planificador_srdf* planificador_srdf_create()
{
	t_planificador_srdf *planif_new = malloc(sizeof(t_planificador_srdf));
	planif_new->listas_y_colas = listas_y_colas_creense();
	planif_new->cola_entrenadores_sin_objetivo=queue_create();
	planif_new->listos_y_ordenados = list_create();
	planif_new->retardo=mapa->info_algoritmo->retardo;
	return planif_new;
}

void planificador_inicia_log()
{
	char *nombre_log = string_new();
	string_append(&nombre_log, "Log ");
	string_append(&nombre_log, mapa->nombre);
	string_append(&nombre_log, " planificador ");
	string_append(&nombre_log, mapa->info_algoritmo->algoritmo);
	informe_planificador =log_create(nombre_log, "Planificador", 0, LOG_LEVEL_INFO);
	free(nombre_log);
}

/*--------------------------------------------DESTROYERS---------------------------------------------------------------*/
void planificador_rr_destruite(t_planificador_rr *rr)
{
	free(rr->listas_y_colas);
	free(rr);
}

void planificador_srdf_destruite(t_planificador_srdf *srdf)
{
	free(srdf->listas_y_colas);
	list_destroy(srdf->listos_y_ordenados);
	queue_destroy(srdf->cola_entrenadores_sin_objetivo);
	free(srdf);
}
/*---------------------------------------PUSH Y POPS DE COLAS---------------------------------------------------------*/
void planificador_push_entrenador_a_bloqueado(t_entrenador *entrenador)
{
	pthread_mutex_lock(&mutex_manipular_cola_bloqueados);
	queue_push(mapa->entrenadores->cola_entrenadores_bloqueados,entrenador);
	int estado_anterior = entrenador->estado;
	mapa_cambiale_estado_a_entrenador(entrenador, BLOQUEADO, estado_anterior);
	pthread_mutex_unlock(&mutex_manipular_cola_bloqueados);
}

void planificador_push_entrenador_a_listo(t_entrenador *entrenador)
{
	pthread_mutex_lock(&mutex_manipular_cola_listos);
	queue_push(mapa->entrenadores->cola_entrenadores_listos, entrenador);

	pthread_mutex_unlock(&mutex_manipular_cola_listos);

}

t_entrenador* planificador_pop_entrenador_listo()
{
	pthread_mutex_lock(&mutex_manipular_cola_listos);
	t_entrenador *entrenador_que_tiene_el_turno = (t_entrenador*) queue_pop(mapa->entrenadores->cola_entrenadores_listos);
	pthread_mutex_unlock(&mutex_manipular_cola_listos);
	return entrenador_que_tiene_el_turno;
}

t_entrenador* planificador_pop_entrenador_listo_to_srdf()
{
	pthread_mutex_lock(&mutex_manipular_cola_listos);

	t_list *list_aux = cola_listos_a_lista(mapa->entrenadores->cola_entrenadores_listos);
	int size = list_size(list_aux);
	int i = 0;
	int entrenador_sin_coordenadas = 0;
	while(i<size && !entrenador_sin_coordenadas)
	{
		t_entrenador *entrenador = list_get(list_aux,i);
		if(!entrenador->tiene_objetivo)
		{
			entrenador_sin_coordenadas = 1;
		}
		i++;
	}
	if(entrenador_sin_coordenadas == 1)
	{
		t_entrenador *entrenador_turno_especial = list_get(list_aux,i-1);
		list_destroy(list_aux);
		pthread_mutex_unlock(&mutex_manipular_cola_listos);
		return entrenador_turno_especial;
	}
	else
	{
		list_destroy(list_aux);
		t_entrenador *entrenador_que_tiene_el_turno = (t_entrenador*) queue_pop(mapa->entrenadores->cola_entrenadores_listos);
		pthread_mutex_unlock(&mutex_manipular_cola_listos);
		return entrenador_que_tiene_el_turno;
	}

}

t_entrenador* planificador_pop_entrenador_bloqueado()
{
	pthread_mutex_lock(&mutex_manipular_cola_bloqueados);
	t_entrenador *entrenador = (t_entrenador *) queue_pop(mapa->entrenadores->cola_entrenadores_bloqueados);
	pthread_mutex_unlock(&mutex_manipular_cola_bloqueados);
	return entrenador;
}

/*-------------------------------------------FUNCIONES GENERALES--------------------------------------------------------*/
void planificador_dale_coordenadas_a_entrenador(t_entrenador *entrenador)
{
	char *identificador_pokenest = escuchar_mensaje_entrenador(entrenador, ENTRENADOR_ESTA_BUSCANDO_COORDENADAS_POKENEST);

	t_pokeNest *pokenest_buscada = mapa_buscame_pokenest(identificador_pokenest);
	if(entrenador->pokenest_objetivo != NULL)
	{
		free(entrenador->pokenest_objetivo);
	}
	entrenador->pokenest_objetivo = identificador_pokenest;
	mapa_actualiza_distancia_del_entrenador(entrenador);
	char *coordendas_pokenest = armar_coordenada(pokenest_buscada->posicion->x,pokenest_buscada->posicion->y, MAX_BYTES_COORDENADA);

	//INICIO LOG
	char *mensaje_A_loggear = string_new();
	string_append(&mensaje_A_loggear, "Entrenador ");
	string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
	string_append(&mensaje_A_loggear, "esta buscando coordenadas de pokenest ");
	string_append(&mensaje_A_loggear, identificador_pokenest);
	t_posicion *posicion = desarmar_coordenada(coordendas_pokenest);

	char *x = string_itoa(posicion->x);
	char *y = string_itoa(posicion->y);
	string_append(&mensaje_A_loggear, " ");
	string_append(&mensaje_A_loggear, "(");
	string_append(&mensaje_A_loggear,x);
	string_append(&mensaje_A_loggear,";");
	string_append(&mensaje_A_loggear,y);
	string_append(&mensaje_A_loggear, ")");

	log_info(informe_planificador, mensaje_A_loggear);
	free(mensaje_A_loggear);
	free(x);
	free(y);
	free(posicion);
	//FIN LOG

	enviar_mensaje_a_entrenador(entrenador, OTORGAR_COORDENADAS_POKENEST, coordendas_pokenest);

	free(coordendas_pokenest);
}

void planificador_entrenador_se_mueve(t_entrenador *entrenador)
{
	char *posicion_destino = escuchar_mensaje_entrenador(entrenador, ENTRENADOR_QUIERE_MOVERSE);
	free(entrenador->posicion_actual);
	entrenador->posicion_actual = desarmar_coordenada(posicion_destino);
	free(posicion_destino);

	//INICIO LOG
	/*char *mensaje_A_loggear = string_new();
	string_append(&mensaje_A_loggear, "Entrenador identificado por ");
	string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
	string_append(&mensaje_A_loggear, " se movio a ");
	char *x = string_itoa(entrenador->posicion_actual->x);
	char *y = string_itoa(entrenador->posicion_actual->y);
	string_append(&mensaje_A_loggear,x);
	string_append(&mensaje_A_loggear,";");
	string_append(&mensaje_A_loggear,y);
	log_info(informe_planificador, mensaje_A_loggear);
	free(mensaje_A_loggear);
	free(x);
	free(y);*/
	//FIN LOG

	mapa_actualiza_distancia_del_entrenador(entrenador);
	mapa_mostra_actualizacion_de_entrenador(entrenador);
}

void planificador_entrenador_quiere_capturar_pokemon(t_entrenador *entrenador, int permiso)
{
	//INICIO LOG
	char *mensaje_A_loggear = string_new();
	string_append(&mensaje_A_loggear, "Entrenador identificado por ");
	string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
	string_append(&mensaje_A_loggear, " esta solicitando captura pokemon");
	log_info(informe_planificador, mensaje_A_loggear);
	free(mensaje_A_loggear);
	//FIN LOG

	planificador_push_entrenador_a_bloqueado(entrenador);
	if(permiso == PERMITIR_SI_ES_POSIBLE)
	{
		planificador_trata_captura_pokemon(entrenador);
	}
	else
	{
		deadlock_actualizar_matriz(entrenador->simbolo_identificador,entrenador->pokenest_objetivo,MATRIZ_SOLICITUD,OTORGAR_RECURSO);
	};
}

void planificador_trata_captura_pokemon(t_entrenador *entrenador)
{
	if(mapa_decime_si_hay_pokemones_en_pokenest(entrenador->pokenest_objetivo))
	{
		planificador_desbloqueame_a(entrenador);
		planificador_entrega_pokemon_a(entrenador);
	}
	else
	{

		//INICIO LOG
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "Entrenador identificado por ");
		string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
		string_append(&mensaje_A_loggear, " quedo bloqueado por no haber pokemons disponibles");
		log_info(informe_planificador, mensaje_A_loggear);
		free(mensaje_A_loggear);
		//FIN LOG
		deadlock_actualizar_matriz(entrenador->simbolo_identificador,entrenador->pokenest_objetivo,MATRIZ_SOLICITUD,OTORGAR_RECURSO);
	}
}

void planificador_entrega_pokemon_a(t_entrenador *entrenador)
{
	char* pokemon_a_entregar = mapa_dame_pokemon_de_pokenest(entrenador->pokenest_objetivo);
	enviar_mensaje_a_entrenador(entrenador,OTORGAR_POKEMON, pokemon_a_entregar);

	if(proceso_porId_tiene_solicitudes(entrenador->simbolo_identificador))
	{
		deadlock_actualizar_matriz(entrenador->simbolo_identificador,entrenador->pokenest_objetivo,MATRIZ_SOLICITUD,QUITAR_RECURSO);
	}
	deadlock_actualizar_matriz(entrenador->simbolo_identificador,entrenador->pokenest_objetivo,MATRIZ_ASIGNACION,OTORGAR_RECURSO);

	char* pokemon_aux = string_new();
	string_append(&pokemon_aux,pokemon_a_entregar);
	list_add(entrenador->pokemones_capturados, pokemon_aux);

	//INICIO LOG
	char *mensaje_A_loggear = string_new();
	//string_append(&mensaje_A_loggear, "Entrenador identificado por ");
	string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
	string_append(&mensaje_A_loggear, " recibio ");
	string_append(&mensaje_A_loggear,pokemon_a_entregar);
	log_info(informe_planificador, mensaje_A_loggear);
	free(mensaje_A_loggear);
	//FIN LOG

	free(pokemon_a_entregar);
	mapa_actualiza_pokemones_disponibles_de_pokenest(entrenador->pokenest_objetivo);
}

void planificador_bloquea_entrenador(t_entrenador *entrenador)
{
	planificador_push_entrenador_a_bloqueado(entrenador);
}

void planificador_desbloqueame_a(t_entrenador *entrenador)
{
	pthread_mutex_lock(&mutex_manipular_cola_bloqueados);
	cola_bloqueados_quita_entrenador_especifico(mapa->entrenadores->cola_entrenadores_bloqueados,entrenador->id_proceso);
	pthread_mutex_unlock(&mutex_manipular_cola_bloqueados);
	mapa_cambiale_estado_a_entrenador(entrenador, EXECUTE, BLOQUEADO);
}

void cola_bloqueados_quita_entrenador_especifico(t_queue *cola, int id_proceso)
{
	t_queue *aux_entrenador = queue_create();
	t_queue *aux = queue_create();
	int tamanio_de_cola = queue_size(cola);
	int i;
	for(i=0; i<tamanio_de_cola; i++)
	{
		t_entrenador *entrenador =queue_pop(cola);
		if(entrenador->id_proceso == id_proceso)
		{
			queue_push(aux_entrenador,entrenador);
		}
		else {queue_push(aux,entrenador);}
	}
	int u;
	for(u=0; u<tamanio_de_cola-1; u++)
	{
		queue_push(cola, queue_pop(aux));
	}
	queue_destroy(aux_entrenador);
	queue_destroy(aux);
}

void planificador_revisa_si_hay_recursos_para_desbloquear_entrenadores()
{
	int cantidad_bloqueados = queue_size(mapa->entrenadores->cola_entrenadores_bloqueados);
	if(cantidad_bloqueados != 0)
	{
		//INICIO LOG
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "Se revisa si se puede desbloquear algún entrenador ");
		log_info(informe_planificador, mensaje_A_loggear);
		free(mensaje_A_loggear);
		//FIN LOG

		planificador_desbloquea_entrenador_si_es_posible(cantidad_bloqueados);

	}
	else {}
}

void planificador_desbloquea_entrenador_si_es_posible(int cantidad_bloqueados)
{
	int i;
	for(i=0; i<cantidad_bloqueados; i++)
	{
		t_entrenador *entrenador =  planificador_pop_entrenador_bloqueado();
		if(mapa_decime_si_hay_pokemones_en_pokenest(entrenador->pokenest_objetivo))
		{
			planificador_push_entrenador_a_listo(entrenador);
		}
		else
		{
			planificador_push_entrenador_a_bloqueado(entrenador);
		}

	}
}

/*---------------------------------------FINALIZADO---------------------------------------------------------*/
void planificador_aborta_entrenador(t_entrenador *entrenador)
{
	char* mensaje_a_log = string_new();
	string_append(&mensaje_a_log,"SE ABORTA AL ENTRENADOR IDENTIFICADO POR: ");
	string_append(&mensaje_a_log,entrenador->simbolo_identificador);
	log_info(informe_planificador, mensaje_a_log);
	free(mensaje_a_log);

	if(proceso_porId_tiene_solicitudes(entrenador->simbolo_identificador))
	{
		deadlock_actualizar_matriz(entrenador->simbolo_identificador,entrenador->pokenest_objetivo,MATRIZ_SOLICITUD,QUITAR_RECURSO);
	}
	mapa_cambiale_estado_a_entrenador(entrenador, MUERTO, EXECUTE);
	entrenador->objetivo_cumplido = ABORTADO; //Ojo
	server_cerra_cliente(entrenador->socket_entrenador);
	char *key = string_itoa(entrenador->socket_entrenador);

	dictionary_put(mapa->entrenadores->lista_entrenadores_finalizados,key, entrenador);
	dictionary_remove(mapa->diccionario_de_entrenadores,entrenador->simbolo_identificador);

	planificador_extraele_pokemones_a_entrenador(entrenador);
	mapa_elimina_entrenador_de_pantalla(entrenador);
	deadlock_elimina_proceso_de_matrices(entrenador->simbolo_identificador);

	free(key);
	free(entrenador->pokenest_objetivo);
	free(entrenador->posicion_actual);
	free(entrenador->simbolo_identificador);
}

void planificador_aborta_entrenador_por_deadlock(t_entrenador* entrenador)
{
		char* mensaje_a_log = string_new();
		string_append(&mensaje_a_log,"SE ABORTA POR SER ELEGIDO COMO VICTIMA AL ENTRENADOR IDENTIFICADO POR: ");
		string_append(&mensaje_a_log,entrenador->simbolo_identificador);
		log_info(informe_planificador, mensaje_a_log);
		free(mensaje_a_log);

		planificador_extraele_pokemones_a_entrenador(entrenador);
		planificador_desbloqueame_a(entrenador);

		if(proceso_porId_tiene_solicitudes(entrenador->simbolo_identificador))
		{
				deadlock_actualizar_matriz(entrenador->simbolo_identificador,entrenador->pokenest_objetivo,MATRIZ_SOLICITUD,QUITAR_RECURSO);
		}
		deadlock_elimina_proceso_de_matrices(entrenador->simbolo_identificador);
		mapa_cambiale_estado_a_entrenador(entrenador, MUERTO, EXECUTE);
		entrenador->objetivo_cumplido = ABORTADO; //Ojo


		char *key = string_itoa(entrenador->socket_entrenador);
		dictionary_put(mapa->entrenadores->lista_entrenadores_finalizados,key, entrenador);
		dictionary_remove(mapa->diccionario_de_entrenadores,entrenador->simbolo_identificador);

		enviar_mensaje_a_entrenador(entrenador,AVISAR_QUE_PERDIO,NULL);
		server_cerra_cliente(entrenador->socket_entrenador);
		mapa_elimina_entrenador_de_pantalla(entrenador);

		free(entrenador->pokenest_objetivo);
		free(entrenador->posicion_actual);
		free(entrenador->simbolo_identificador);
}

void planificador_finaliza_entrenador(t_entrenador *entrenador)
{
	mapa_cambiale_estado_a_entrenador(entrenador, MUERTO, EXECUTE);
	entrenador->objetivo_cumplido = CUMPLIDO;
	planificador_espera_que_entrenador_se_desconecte(entrenador);
	mapa_borra_entrenador_de_pantalla(entrenador);
}

void planificador_espera_que_entrenador_se_desconecte(t_entrenador *entrenador)
{
	int payloadSize = 3;
	char * payload = malloc(payloadSize);
	int resultado = recv(entrenador->socket_entrenador, payload, payloadSize,0);

	if(resultado == DESCONECTADO)
	{
		server_cerra_cliente(entrenador->socket_entrenador);
		char *key = string_itoa(entrenador->socket_entrenador);
		dictionary_put(mapa->entrenadores->lista_entrenadores_finalizados,key, entrenador);
		free(payload);
		planificador_extraele_pokemones_a_entrenador(entrenador);
	}
}

void planificador_extraele_pokemones_a_entrenador(t_entrenador *entrenador)
{
	int cantidad_pokemones_que_tenia = list_size(entrenador->pokemones_capturados);
	if(cantidad_pokemones_que_tenia != 0)
	{
		int i;
		for(i=0; i<cantidad_pokemones_que_tenia; i++)
		{
			char* pokemon_a_devolver = list_get(entrenador->pokemones_capturados,i);
			mapa_devolve_pokemon_a_pokenest(pokemon_a_devolver);
		}
	}
	list_destroy(entrenador->pokemones_capturados);
}

/*---------------------------------------NUEVO->LISTO---------------------------------------------------------*/

void* planificador_encola_nuevos_entrenadores()
{
		if(mapa_decime_si_planificador_es(PLANIFICADOR_RR))
		{
			pthread_mutex_lock(&mutex_manipular_cola_nuevos);
			foreach(COLA_LISTOS,mapa->entrenadores->lista_entrenadores_a_planificar,planificador_modela_nuevo_entrenador_y_encolalo);
			if(!hay_jugadores_online)
			{
				hay_jugadores_online=1;
				sem_post(&semaforo_entrenadores_listos);
			}

			list_clean(mapa->entrenadores->lista_entrenadores_a_planificar);
			pthread_mutex_unlock(&mutex_manipular_cola_nuevos);
		}
		else
		{
			pthread_mutex_lock(&mutex_manipular_cola_nuevos);
			foreach(COLA_SIN_OBJETIVOS,mapa->entrenadores->lista_entrenadores_a_planificar,planificador_modela_nuevo_entrenador_y_encolalo);
			if(!hay_jugadores)
			{
				hay_jugadores = SI;
				sem_post(&semaforo_esperar_por_entrenador_listo);
			}
			list_clean(mapa->entrenadores->lista_entrenadores_a_planificar);
			pthread_mutex_unlock(&mutex_manipular_cola_nuevos);

			/*pthread_mutex_lock(&mutex_manipular_cola_nuevos);
						foreach(COLA_SIN_OBJETIVOS,mapa->entrenadores->lista_entrenadores_a_planificar,planificador_modela_nuevo_entrenador_y_encolalo);
						list_clean(mapa->entrenadores->lista_entrenadores_a_planificar);
						pthread_mutex_unlock(&mutex_manipular_cola_nuevos);*/
		}

}

void planificador_modela_nuevo_entrenador_y_encolalo(int cola, void *entrenador)
{
	t_entrenador_nuevo *entrenador_a_modelar = (t_entrenador_nuevo *) entrenador;
	t_entrenador *new_entrenador = entrenador_create(entrenador_a_modelar->id_proceso, entrenador_a_modelar->socket_entrenador);
	new_entrenador->simbolo_identificador = entrenador_a_modelar->simbolo_identificador;
	free(entrenador_a_modelar);
	mapa_mostra_nuevo_entrenador_en_pantalla(new_entrenador);

	dictionary_put(mapa->diccionario_de_entrenadores,new_entrenador->simbolo_identificador,new_entrenador); // AGREGADO

	if(cola==COLA_LISTOS)
	{
		planificador_push_entrenador_a_listo(new_entrenador);
		deadlock_agregar_nuevo_proceso_a_matrices(new_entrenador->simbolo_identificador);
	}
	else
	{
		//planificador_push_entrenador_en_cola_sin_objetivos(new_entrenador);
		planificador_push_entrenador_a_listo(new_entrenador);
		deadlock_agregar_nuevo_proceso_a_matrices(new_entrenador->simbolo_identificador);
	}

}

void foreach(int cola,void *lista,void(*funcion_de_lista)(int,void*))
{
	t_list *lista_a_planificar = (t_list*)lista;
	int tamanio = list_size(lista);
	int i;
	for(i=0; i<tamanio;i++)
	{
		funcion_de_lista(cola,list_get(lista_a_planificar, i));
	}
}

/*---------------------------------------EXECUTE->LISTO---------------------------------------------------------*/
void planificador_volve_a_encolar_a_listo_si_es_necesario(t_entrenador *entrenador)
{
	if(mapa_decime_si_entrenador_esta_bloqueado(entrenador) || mapa_decime_si_entrenador_esta_abortado(entrenador)|| entrenador->estado==MUERTO)
	{

	}
	else
	{
		planificador_push_entrenador_a_listo(entrenador);
	}
}


/*---------------------------------------LOGS---------------------------------------------------------*/
void mostrarTodo(t_queue* cola, int tipo)
{
	int z;
	char* mensaje = string_new();
	string_append(&mensaje,"COLA ");

	int tamanio_cola = queue_size(cola);
	switch(tipo)
	{
		case(COLA_BLOQUEADOS):
		{
			string_append(&mensaje,"BLOQUEADOS: ");
			if(tamanio_cola == 0)
				{
					string_append(&mensaje,"VACIA");
				}
			else
			{
				pthread_mutex_lock(&mutex_manipular_cola_bloqueados);
					for(z=0; z < tamanio_cola; z++)
					{
						t_entrenador *entrener = list_get(cola->elements,z);
						string_append(&mensaje,entrener->simbolo_identificador);
						string_append(&mensaje," ");
					}
					pthread_mutex_unlock(&mutex_manipular_cola_bloqueados);
			}
			log_info(informe_planificador,mensaje);
			free(mensaje);
		};break;
		case(COLA_LISTOS):
		{
			string_append(&mensaje,"LISTOS: ");
			if(tamanio_cola == 0)
			{
				string_append(&mensaje,"VACIA");
			}
			else
			{
				pthread_mutex_lock(&mutex_manipular_cola_listos);
				for(z=0; z < tamanio_cola; z++)
				{

					t_entrenador *entrener = list_get(cola->elements,z);
					string_append(&mensaje,entrener->simbolo_identificador);
					string_append(&mensaje," ");
				}
				pthread_mutex_unlock(&mutex_manipular_cola_listos);
			}
			log_info(informe_planificador,mensaje);
			free(mensaje);
		};break;
	}
}

char* convertir_a_string(char caracter)
{
	char buffer[2];
	buffer[0] = caracter;
	buffer[1] = '\0';
	char* retorno = string_new();
	string_append(&retorno,buffer);
	return retorno;
}
