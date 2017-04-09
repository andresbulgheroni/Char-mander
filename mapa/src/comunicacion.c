/*
 * comunicacion.c
 *
 *  Created on: 12/9/2016
 *      Author: utnso
 */

#include "comunicacion.h"

pthread_mutex_t  mutex_escuchar_al_entrenador = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  mutex_enviar_al_entrenador = PTHREAD_MUTEX_INITIALIZER;

void* ejecutar_servidor(void *argumento)
{
	t_info_socket *info_sock = (t_info_socket*) argumento;

	t_server *server_pthread = server_create(info_sock->puerto,info_sock->ip, BACKLOG);
	server_escucha(server_pthread);
	int se_puede_ejecutar = 1;

	while(se_puede_ejecutar > 0)
	{
		int conexion = server_acepta_conexion_cliente(server_pthread);
		if(conexion == SERVER_DESCONECTADO)
		{
			se_puede_ejecutar = 0;
		}
		else {conexion_create(&conexion);}
	}

}

void conexion_create(int *conexion)
{
	pthread_attr_t attr;
	pthread_t thread;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&thread,&attr,atender_cliente,(void*)conexion);

	pthread_attr_destroy(&attr);

}

void* atender_cliente(void* argumento)
{
	int *conexion = (int*) argumento;
	agregar_proceso_a_lista(conexion,NULL);
	pthread_exit(NULL);
}

void agregar_proceso_a_lista(int *socket_cliente, sem_t *semaforo_finalizacion)
{
	t_entrenador_nuevo *entrenador = malloc(sizeof(t_entrenador_nuevo));
	entrenador->id_proceso = (int)process_get_thread_id();
	entrenador->socket_entrenador = *socket_cliente;
	entrenador->simbolo_identificador = recibir_mensaje(*socket_cliente,1);

	pthread_mutex_lock(&mutex_manipular_cola_nuevos);
	list_add(mapa->entrenadores->lista_entrenadores_a_planificar,entrenador);
	pthread_mutex_unlock(&mutex_manipular_cola_nuevos);

	//INICIO LOG
	char *mensaje_A_loggear = string_new();
	string_append(&mensaje_A_loggear, "PUSH (NUEVO) entrenador identificado con el simbolo ");
	string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
	string_append(&mensaje_A_loggear, " y por el socket ");
	char* num_aux = string_itoa(entrenador->socket_entrenador);
	string_append(&mensaje_A_loggear,num_aux);
	log_info(informe_planificador, mensaje_A_loggear);
	free(mensaje_A_loggear);
	free(num_aux);
	//FIN LOG

	//sem_post(&semaforo_hay_algun_entrenador_listo);
	planificador_encola_nuevos_entrenadores();
}

/*-------------------------------------------DECODIFICACION DE RESPUESTAS------------------------------------------------*/
int tratar_respuesta(char* respuesta_del_entrenador, t_entrenador *entrenador)
{
	if(string_equals_ignore_case(respuesta_del_entrenador, "up;"))
	{
		return ENTRENADOR_ESTA_BUSCANDO_COORDENADAS_POKENEST;
	}
	if(string_equals_ignore_case(respuesta_del_entrenador,"mp;"))
	{
		return ENTRENADOR_QUIERE_MOVERSE;
	}
	if(string_equals_ignore_case(respuesta_del_entrenador, "cp;"))
	{
		return ENTRENADOR_QUIERE_CAPTURAR_POKEMON;
	}
	if(string_equals_ignore_case(respuesta_del_entrenador, "fp;"))
	{
		return ENTRENADOR_FINALIZO_OBJETIVOS;
	}
	if(string_equals_ignore_case(respuesta_del_entrenador,"DESCONECTADO"))
	{
		return ENTRENADOR_DESCONECTADO;
	}
	if(string_equals_ignore_case(respuesta_del_entrenador,"123"))
	{
		return ENTRENADOR_SIGUE_VIVO;
	}
	else {return 0;}
}

void enviar_mensaje_a_entrenador(t_entrenador *entrenador, int header, char *payload)
{
	switch(header)
	{
		case(OTORGAR_TURNO):enviar_mensaje(entrenador->socket_entrenador, "tr;"); break;
		case(OTORGAR_COORDENADAS_POKENEST):
		{
			char* mensaje = armar_mensaje("ur",payload,MAX_BYTES_TOTAL_A_ENVIAR);
			enviar_mensaje(entrenador->socket_entrenador, mensaje);
			free(mensaje);
		};break;
		case(OTORGAR_MEDALLA_DEL_MAPA): otorgar_ruta_medalla_a_entrenador(entrenador->socket_entrenador, mapa_dame_medalla()); break;
		case(OTORGAR_POKEMON): dar_pokemon_a_entrenador(entrenador, payload);break;
		case(AVISAR_BLOQUEO_A_ENTRENADOR): enviar_mensaje(entrenador->socket_entrenador, "bq;");  break;
		case(AVISAR_DESBLOQUEO_A_ENTRENADOR): enviar_mensaje(entrenador->socket_entrenador,"fb;"); break;
		case(AVISAR_DEADLOCK): enviar_mensaje(entrenador->socket_entrenador, "mpk");break;
		case(AVISAR_QUE_GANO): enviar_mensaje(entrenador->socket_entrenador, "gnr");break;
		case(AVISAR_QUE_PERDIO): enviar_mensaje(entrenador->socket_entrenador, "prd");break;
		case(PREGUNTAR_SI_SIGUE_AHI): enviar_mensaje(entrenador->socket_entrenador,"123");break;
		default: ;
	}
}

char* escuchar_mensaje_entrenador(t_entrenador *entrenador, int header)
{
	switch(header)
	{
		case(SOLICITUD_DEL_ENTRENADOR): return(recibir_mensaje(entrenador->socket_entrenador,3));
		case(ENTRENADOR_OTORGA_SU_SIMBOLO): return(recibir_mensaje_especifico(entrenador->socket_entrenador)); break;
		case(ENTRENADOR_ESTA_BUSCANDO_COORDENADAS_POKENEST): return(recibir_mensaje_especifico(entrenador->socket_entrenador)); break;
		case(ENTRENADOR_QUIERE_MOVERSE): return(recibir_mensaje_especifico(entrenador->socket_entrenador)); break;
		default: return("0"); break;
	}
}

void otorgar_ruta_medalla_a_entrenador(int entrenador, char *rutaMedalla)
{
	char *mensaje = armar_mensaje("mr", rutaMedalla,MAX_BYTES_TOTAL_A_ENVIAR);
	enviar_mensaje(entrenador, mensaje);
	free(mensaje);
	free(rutaMedalla);
}

void dar_pokemon_a_entrenador(t_entrenador *entrenador, char *ruta_pokemon)
{
	char *mensaje = armar_mensaje("sr",ruta_pokemon, MAX_BYTES_TOTAL_A_ENVIAR);
	enviar_mensaje(entrenador->socket_entrenador, mensaje);
	free(mensaje);
}

t_posicion* desarmar_coordenada(char *coordenada)
{
	char **por_separado = string_split(coordenada, ";");
	string_trim_left(&por_separado[0]);
	string_trim_left(&por_separado[1]);
	t_posicion  *posicion = posicion_create(atoi(por_separado[0]),atoi(por_separado[1]));
	array_free_all(por_separado);
	return posicion;

}

void* escuchar_mejor_pokemon(int socket_entrenador)
{
	t_pkmn_factory *factory = create_pkmn_factory();

	char* tamanio_specie = recibir_mensaje(socket_entrenador,14);
	if(!string_equals_ignore_case(tamanio_specie, "DESCONECTADO"))
	{
		int tamanio_specie_numero = atoi(tamanio_specie);
		free(tamanio_specie);

			char* specie = recibir_mensaje(socket_entrenador,tamanio_specie_numero);
			if(!string_equals_ignore_case(specie,"DESCONECTADO"))
			{
				char* level_string = recibir_mensaje(socket_entrenador,6);
				if(!string_equals_ignore_case(level_string, "DESCONECTADO"))
				{
					int level = atoi(level_string);
					free(level_string);
					t_pokemon *new_pokemon = create_pokemon(factory,specie,level);

					destroy_pkmn_factory(factory);
					return new_pokemon;
				}
				else
				{
					destroy_pkmn_factory(factory);
					return "DESCONECTADO";
				}

			}
			else
			{
				destroy_pkmn_factory(factory);
				return "DESCONECTADO";
			}
	}
	else
	{
		destroy_pkmn_factory(factory);
		return "DESCONECTADO";
	}
}
