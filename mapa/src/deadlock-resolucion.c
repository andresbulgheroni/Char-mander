/*
 * deadlock-resolucion.c
 *
 *  Created on: 11/11/2016
 *      Author: utnso
 */
#include "deadlock-resolucion.h"

extern t_log* logger;

void resolver_deadlock_si_es_posible(t_list* involucrados)
{
	if(mapa->batalla)
	{
		resolver_deadlock_pokemon(involucrados);
	}
	else
	{
		resolver_deadlock_sin_batalla(involucrados);

	}
}

void resolver_deadlock_sin_batalla(t_list *involucrados)
{
	log_info(logger, "EL MODO BATALLA NO ESTA ACTIVADO");
	t_list *entrenadores_involucrados = recuperar_entrenadores_involucrados(involucrados);
	list_destroy_and_destroy_elements(involucrados,free_names_dir);

	int size = list_size(entrenadores_involucrados);
	int i=0;
	int encontre_desconectado = 0;
	while(i<size && !encontre_desconectado)
	{
		t_entrenador *entrenador = list_get(entrenadores_involucrados,i);
		enviar_mensaje_a_entrenador(entrenador,PREGUNTAR_SI_SIGUE_AHI,NULL);
		char* respuesta_string = escuchar_mensaje_entrenador(entrenador,SOLICITUD_DEL_ENTRENADOR);
		int respuesta_posta = tratar_respuesta(respuesta_string,entrenador);
		free(respuesta_string);
		if(respuesta_posta == ENTRENADOR_DESCONECTADO)
		{
			planificador_desbloqueame_a(entrenador);
			planificador_aborta_entrenador(entrenador);
			encontre_desconectado=1;
		}

		i++;
	}
	list_destroy(entrenadores_involucrados);
	cambiar_semaforos_si_es_necesario();
}

void resolver_deadlock_pokemon(t_list* involucrados)
{
	//if(mapa->batalla)
	//{
		int index_entrenador = 0;

		log_info(logger, "SE COMIENZA A RESOLVER EL DEADLOCK");
		t_list *entrenadores_involucrados = recuperar_entrenadores_involucrados(involucrados);

		list_destroy_and_destroy_elements(involucrados,free_names_dir);

		ordenar_entrenadores_segun_tiempo_de_ingreso(entrenadores_involucrados);

		t_entrenador* entrenador_1 = (t_entrenador*) list_get(entrenadores_involucrados,index_entrenador);
		index_entrenador++;
		entrenador_1->esta_en_deadlock = SI;


		t_entrenador* entrenador_2 = (t_entrenador*) list_get(entrenadores_involucrados,index_entrenador);
		index_entrenador++;
		entrenador_2->esta_en_deadlock=SI;

		char* mensaje_a_log = string_new();
		string_append(&mensaje_a_log,"LOS ENTRENADORES ELEGIDOS PARA LA BATALLA SON: ");
		string_append(&mensaje_a_log,entrenador_1->simbolo_identificador);
		string_append(&mensaje_a_log," Y: ");
		string_append(&mensaje_a_log,entrenador_2->simbolo_identificador);
		log_info(logger,mensaje_a_log);
		free(mensaje_a_log);


		t_perdedor *perdedor;
		perdedor = efectuar_batalla_pokemon_entre(entrenador_1,entrenador_2);

		if(perdedor->caso == POR_DESCONEXION)
		{

			char* mensaje_a_log_2 = string_new();
			string_append(&mensaje_a_log_2,"EL PERDEDOR FUE: ");
			string_append(&mensaje_a_log_2,perdedor->victima->simbolo_identificador);
			string_append(&mensaje_a_log_2, " POR DESCONEXION");
			log_info(logger, mensaje_a_log_2);
			free(mensaje_a_log_2);

			planificador_desbloqueame_a(perdedor->victima);
			planificador_aborta_entrenador(perdedor->victima);
			cambiar_semaforos_si_es_necesario();
		}
		else
		{
			char* mensaje_a_log_2 = string_new();
			string_append(&mensaje_a_log_2,"EL PERDEDOR FUE: ");
			string_append(&mensaje_a_log_2,perdedor->victima->simbolo_identificador);
			string_append(&mensaje_a_log_2, " con su pokemon ");
			string_append(&mensaje_a_log_2, perdedor->pokemon_de_la_victima->species);
			log_info(logger, mensaje_a_log_2);
			free(mensaje_a_log_2);

			int hay_victima_por_desconexion = 0;
			int size = list_size(entrenadores_involucrados);

			/* SE COMIENZA LA BATALLA ANIDADA HASTA QUE ALGUIEN SE DESCONECTE O HASTA QUE ALGUNO PIERDA DEFINITIVAMENTE */
			while(index_entrenador<size && !hay_victima_por_desconexion)
			{
				t_entrenador* siguiente_entrenador = (t_entrenador*) list_get(entrenadores_involucrados,index_entrenador);
				index_entrenador++;

				siguiente_entrenador->esta_en_deadlock = SI;

				char* mensaje_a_log = string_new();
				string_append(&mensaje_a_log,"SE HARÁ OTRA BATALLA Y LOS LOS ENTRENADORES ELEGIDOS SON: ");
				string_append(&mensaje_a_log,siguiente_entrenador->simbolo_identificador);
				string_append(&mensaje_a_log," Y ");
				string_append(&mensaje_a_log,perdedor->victima->simbolo_identificador);
				log_info(logger,mensaje_a_log);
				free(mensaje_a_log);

				perdedor = efectuar_batalla_pokemon_con_el_anterior_perdedor(perdedor,siguiente_entrenador);

				if(perdedor->caso == POR_DESCONEXION)
				{
					hay_victima_por_desconexion = 1;
				}

				char* mensaje_a_log_2 = string_new();
				string_append(&mensaje_a_log_2,"EL PERDEDOR FUE: ");
				string_append(&mensaje_a_log_2,perdedor->victima->simbolo_identificador);
				log_info(logger, mensaje_a_log_2);
				free(mensaje_a_log_2);
			}
			planificador_aborta_entrenador_por_deadlock(perdedor->victima);
			cambiar_semaforos_si_es_necesario();
			free(perdedor);
			list_destroy(entrenadores_involucrados);
		}
	//}
}

void involucrados_destroyer(void* arg)
{
	char* value = (char*) arg;
	free(arg);
}

void cambiar_semaforos_si_es_necesario()
{
	if(mapa_decime_si_planificador_es(PLANIFICADOR_RR))
	{
		//semaforo_rr_cambiado_por_deadlock = 1;
		planificador_rr_cambia_semaforo_si_es_necesario();
	}
	else
	{
		//semaforo_srdf_cambiado_por_deadlock = 1;
		planificador_srdf_cambia_semaforo_si_es_necesario();
	}
}

t_perdedor* efectuar_batalla_pokemon_con_el_anterior_perdedor(t_perdedor* anterior_perdedor, t_entrenador *entrenador)
{
	enviar_mensaje_a_entrenador(entrenador,AVISAR_DEADLOCK,NULL);
	void* respuesta_entrenador = escuchar_mejor_pokemon(entrenador->socket_entrenador);

	t_perdedor* new_perdedor = malloc(sizeof(t_perdedor));


	if(string_equals_ignore_case((char*) respuesta_entrenador, "DESCONECTADO"))
	{
		enviar_mensaje_a_entrenador(anterior_perdedor->victima,AVISAR_QUE_GANO,NULL);
		anterior_perdedor->victima->esta_en_deadlock=NO;
		new_perdedor->caso = POR_DESCONEXION;
		new_perdedor->victima = anterior_perdedor->victima;
		new_perdedor->pokemon_de_la_victima =anterior_perdedor->pokemon_de_la_victima;
		free(anterior_perdedor);
		return new_perdedor;
	}
	else
	{
		char* mensaje_a_log = string_new();
		string_append(&mensaje_a_log,"Recibi mejor pokemon de: ");
		string_append(&mensaje_a_log,entrenador->simbolo_identificador);
		log_info(logger, mensaje_a_log);
		free(mensaje_a_log);


		t_pokemon *pokemon_entrenador = (t_pokemon*) respuesta_entrenador;

		char* mensaje_a_log_2 = string_new();
		string_append(&mensaje_a_log_2,"COMIENZA BATALLA ENTRE: ");
		string_append(&mensaje_a_log_2,anterior_perdedor->pokemon_de_la_victima->species);
		string_append(&mensaje_a_log_2," Y ");
		string_append(&mensaje_a_log_2,pokemon_entrenador->species);
		log_info(logger, mensaje_a_log_2);
		free(mensaje_a_log_2);

		t_pokemon *pokemon_perdedor = pkmn_battle(anterior_perdedor->pokemon_de_la_victima,pokemon_entrenador);
		log_info(logger, "TERMINÓ BATALLA!");

		int resultado = retornar_perdedor(pokemon_perdedor,anterior_perdedor->pokemon_de_la_victima, pokemon_entrenador);

		if(resultado == VICTIMA_ENTRENADOR_1)
		{
			enviar_mensaje_a_entrenador(entrenador,AVISAR_QUE_GANO,NULL);
			entrenador->esta_en_deadlock=NO;
			new_perdedor->caso = POR_BATALLA;
			new_perdedor->victima = anterior_perdedor->victima;
			new_perdedor->pokemon_de_la_victima = anterior_perdedor->pokemon_de_la_victima;
			free(anterior_perdedor);
			destroy_pokemon(pokemon_entrenador);
			return new_perdedor;
		}
		else
		{
			enviar_mensaje_a_entrenador(anterior_perdedor->victima,AVISAR_QUE_GANO,NULL);
			anterior_perdedor->victima->esta_en_deadlock=NO;
			new_perdedor->caso = POR_BATALLA;
			new_perdedor->victima= entrenador;
			new_perdedor->pokemon_de_la_victima = pokemon_entrenador;
			free(anterior_perdedor);
			return new_perdedor;
		}
	}
}

t_perdedor* efectuar_batalla_pokemon_entre(t_entrenador *entrenador_1, t_entrenador *entrenador_2)
{
	enviar_mensaje_a_entrenador(entrenador_1,AVISAR_DEADLOCK,NULL);
	void* respuesta_entrenador_1 = escuchar_mejor_pokemon(entrenador_1->socket_entrenador);

	enviar_mensaje_a_entrenador(entrenador_2,AVISAR_DEADLOCK,NULL);
	void* respuesta_entrenador_2 = escuchar_mejor_pokemon(entrenador_2->socket_entrenador);

	t_perdedor* new_perdedor = malloc(sizeof(t_perdedor));
	new_perdedor->pokemon_de_la_victima = malloc(sizeof(t_pokemon));


	if(string_equals_ignore_case((char*) respuesta_entrenador_1, "DESCONECTADO"))
	{
		enviar_mensaje_a_entrenador(entrenador_2,AVISAR_QUE_GANO,NULL);
		entrenador_2->esta_en_deadlock = NO;
		new_perdedor->caso = POR_DESCONEXION;
		new_perdedor->victima = entrenador_1;
		return new_perdedor;
	}
	else
	{
		if(string_equals_ignore_case((char*) respuesta_entrenador_2, "DESCONECTADO"))
		{
			enviar_mensaje_a_entrenador(entrenador_1,AVISAR_QUE_GANO,NULL);
			entrenador_1->esta_en_deadlock=NO;
			new_perdedor->caso = POR_DESCONEXION;
			new_perdedor->victima = entrenador_2;
			return new_perdedor;
		}
		else
		{
			/* NINGNO SE DESCONECTO! HACEMOS LA BATALLA */
			t_pokemon *pokemon_entrenador_1 = (t_pokemon*) respuesta_entrenador_1;
			t_pokemon *pokemon_entrenador_2 = (t_pokemon*) respuesta_entrenador_2;

			char* mensaje_a_log_2 = string_new();
			string_append(&mensaje_a_log_2,"COMIENZA BATALLA ENTRE: ");
			string_append(&mensaje_a_log_2,pokemon_entrenador_1->species);
			string_append(&mensaje_a_log_2," Y ");
			string_append(&mensaje_a_log_2,pokemon_entrenador_2->species);
			log_info(logger, mensaje_a_log_2);
			free(mensaje_a_log_2);

			t_pokemon *pokemon_perdedor = pkmn_battle(pokemon_entrenador_1,pokemon_entrenador_2);

			int perdedor = retornar_perdedor(pokemon_perdedor,pokemon_entrenador_1,pokemon_entrenador_2);

			if(perdedor == VICTIMA_ENTRENADOR_1)
			{
				enviar_mensaje_a_entrenador(entrenador_2,AVISAR_QUE_GANO,NULL);
				entrenador_2->esta_en_deadlock = NO;
				new_perdedor->caso = POR_BATALLA;
				new_perdedor->victima = entrenador_1;
				memcpy(new_perdedor->pokemon_de_la_victima,pokemon_entrenador_1,sizeof(t_pokemon));
				//new_perdedor->pokemon_de_la_victima = pokemon_entrenador_1;
				destroy_pokemon(pokemon_entrenador_2);
			}
			else
			{
				enviar_mensaje_a_entrenador(entrenador_1,AVISAR_QUE_GANO,NULL);
				entrenador_1->esta_en_deadlock=NO;
				new_perdedor->caso = POR_BATALLA;
				new_perdedor->victima = entrenador_2;
				memcpy(new_perdedor->pokemon_de_la_victima,pokemon_entrenador_2,sizeof(t_pokemon));
				destroy_pokemon(pokemon_entrenador_1);
			}
			return new_perdedor;
		}
	}
}

void destroy_pokemon(t_pokemon *pokemon)
{
	free(pokemon->species);
	free(pokemon);
}

int retornar_perdedor(t_pokemon *perdedor, t_pokemon *pokemon_entrenador_1, t_pokemon *pokemon_entrenador_2)
{
	if(pokemones_son_iguales(perdedor,pokemon_entrenador_1))
	{
		return VICTIMA_ENTRENADOR_1;
	}
	else
	{
		return VICTIMA_ENTRENADOR_2;
	}

}

int pokemones_son_iguales(t_pokemon *pok_1, t_pokemon *pok_2)
{
	if(pok_1->level == pok_2->level && pok_1->second_type== pok_2->second_type && string_equals_ignore_case(pok_1->species,pok_2->species) && pok_1->type == pok_2->type)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

t_list* recuperar_entrenadores_involucrados(t_list* identificadores_involucrados)
{
	int size = list_size(identificadores_involucrados);
	t_list* new_list = list_create();
	int i;
	for(i=0;i<size;i++)
	{
		char* id = list_get(identificadores_involucrados,i);
		t_entrenador *entrenador = (t_entrenador*) dictionary_get(mapa->diccionario_de_entrenadores,id);
		list_add(new_list, entrenador);

		char* mensaje_a_log = string_new();
		string_append(&mensaje_a_log,"EL ENTRENADOR ");
		string_append(&mensaje_a_log,entrenador->simbolo_identificador);
		string_append(&mensaje_a_log," SE ENCUENTRA EN DEADLOCK");
		log_info(logger,mensaje_a_log);
		free(mensaje_a_log);
	}
	return new_list;
}

void ordenar_entrenadores_segun_tiempo_de_ingreso(t_list* entrenadores)
{
	list_sort(entrenadores,comparador_para_batalla);
}

int comparador_para_batalla(void* elem1, void* elem2)
{
	t_entrenador *entrenador_1 = (t_entrenador*) elem1;
	t_entrenador *entrenador_2=(t_entrenador*) elem2;

	if(entrenador_1->numero_de_ingreso > entrenador_2->numero_de_ingreso)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
