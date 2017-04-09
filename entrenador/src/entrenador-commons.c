/*
 * entrenador-commons.c
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */
#include "entrenador-commons.h"


sem_t turno_entrenador;
int matan_al_entrenador = 0;
int se_reintenta=0;

void ejecutar_entrenador(char *nombre_entrenador, char *ruta_pokedex)
{
	hora_de_inicio=string_new();
	hora_de_fin=string_new();
	iniciar_log(nombre_entrenador);
	entrenador_registra_hora(INICIO);
	sem_init(&turno_entrenador,0,0);
	entrenador = entrenador_create(nombre_entrenador, ruta_pokedex);
	entrenador_iniciar_seniales();
	log_info(info_entrenador, "Entrenador creado con EXITO");
	entrenador_comenza_a_explorar();
	entrenador_registra_hora(FIN);
	mostrar_por_pantalla_resultados();
	free(hora_de_fin);
	free(hora_de_inicio);
	entrenador_destruite(entrenador);
}

void mostrar_por_pantalla_resultados()
{
	if(!se_reintenta)
	{
		printf("Logre terminar mi aventura! Llamame %s MAESTRO POKEMON! \n",entrenador->nombre);
	}
	else
	{
		printf("%s :NO LOGRE TERMINAR MI AVENTURA \n",entrenador->nombre);
	}
	//printf("-TIEMPO TOTAL AVENTURA %d \n", entrenador->tiempo_total_aventura);
	printf("-TIEMPO BLOQUEADO EN POKENEST: %d ms. \n", entrenador->tiempo_bloqueado_pokenest);
	printf("-HORA DE INICIO: %s\n", hora_de_inicio);
	printf("-HORA DE FIN: %s\n", hora_de_fin);
	printf("-CANTIDAD DE DEADLOCKS: %d \n", entrenador->cantidad_deadlocks);
	printf("-CANTIDAD DE MUERTES: %d \n", entrenador->muertes);
	printf("-CANTIDAD DE REINTENTOS: %d \n",entrenador->reintentos);
}

void entrenador_iniciar_seniales()
{
	signal(SIGUSR1, subirvida);
	signal(SIGTERM, bajarvida);
	signal(SIGINT, matar_entrenador);
}

void subirvida(int n)
{
	entrenador->vidas++;
	log_info(info_entrenador,"RECIBI MAS VIDA POR SEÑAL EXTERNA");

}

void bajarvida(int n)
{
	entrenador->vidas--;
	if(n != 259)
	{
		log_info(info_entrenador,"PIERDO VIDA POR SEÑAL EXTERNA");
		if(entrenador->vidas <= 0)
		{
			log_info(info_entrenador,"MUERTO POR QUITARME LA ULTIMA VIDA!");
			close(entrenador->mapa_actual->server);
			matan_al_entrenador = 1;
			//entrenador_destruite(entrenador);
		}
	}
}

void entrenador_finalizo_muriendo()
{
	printf("INTENTOS TOTALES: %d \n", entrenador->reintentos);
	printf("GAME OVER. ¿Desea reintentar? Y/N \n");
	tratar_respuesta();
}

void tratar_respuesta()
{
	fflush(stdin);
	char resp;
	scanf("%c", &resp);
	setbuf(stdin,NULL);
	if(resp == 'Y')
	{
		log_info(info_entrenador,"Se reintentara jugar\n");
		entrenador->reintentos = entrenador->reintentos +1;
		entrenador_borra_medallas();
		entrenador_borra_pokemons();
		entrenador_resetea_ubicacion();
		matan_al_entrenador = 0;
		entrenador_comenza_a_explorar();
	}
	else if(resp == 'N')
	{
		log_info(info_entrenador,"Se decidio no jugar mas\n");
		entrenador_borra_medallas();
		entrenador_borra_pokemons();
		se_reintenta=1;
	}
	else{
		printf("Respuesta invalida\n");
		entrenador_finalizo_muriendo();
	}
}

void matar_entrenador(int n)
{
	matan_al_entrenador = 1;
	close(entrenador->mapa_actual->server);
	printf("\n NO LOGRE TERMINAR MI AVENTURA PORQUE ME FINALIZARON \n");
	log_destroy(info_entrenador);
	entrenador_borra_medallas();
	entrenador_borra_pokemons();
	entrenador_destruite(entrenador);
	exit(1);
}

void entrenador_borra_pokemons()
{
	char *ruta_medallas_destino = obtener_ruta_especifica(entrenador->ruta_pokedex, "Entrenadores", entrenador->nombre);
	char* ruta_medallas_destino_full = obtener_ruta_especifica(ruta_medallas_destino,"Dir de Bill",NULL);
	string_append(&ruta_medallas_destino_full,"/");

	borrar_todos_los_archivos_del_directorio(ruta_medallas_destino_full);

	free(ruta_medallas_destino);
	free(ruta_medallas_destino_full);
}

void entrenador_borra_medallas()
{
	char *ruta_medallas_destino = obtener_ruta_especifica(entrenador->ruta_pokedex, "Entrenadores", entrenador->nombre);
	char* ruta_medallas_destino_full = obtener_ruta_especifica(ruta_medallas_destino,"medallas",NULL);
	string_append(&ruta_medallas_destino_full,"/");

	borrar_todos_los_archivos_del_directorio(ruta_medallas_destino_full);
	free(ruta_medallas_destino);
	free(ruta_medallas_destino_full);
}

void iniciar_log(char *nombre_del_entrenador)
{
	char *nombre_log = string_new();
	string_append(&nombre_log, "Log ");
	string_append(&nombre_log, nombre_del_entrenador);
	info_entrenador =log_create(nombre_log, "Proceso entrenador", 1, LOG_LEVEL_INFO);
	free(nombre_log);
}

void entrenador_registra_hora(int rango)
{
	switch(rango)
	{
	case(INICIO):
			{

				char* tiempo= temporal_get_string_time();
				string_append(&hora_de_inicio,tiempo);
				free(tiempo);

			}break;
	case(FIN):
			{

				char* tiempo = temporal_get_string_time();
				string_append(&hora_de_fin,tiempo);
				free(tiempo);

			}break;
	}

}

/*--------------------------------------------LOGICA DE CUMPLIR LOS OBJETIVOS DE TODOS LOS MAPAS---------------------------------*/
void entrenador_comenza_a_explorar()
{
	entrenador_recorre_hoja_de_viaje(NULL);
}

void entrenador_recorre_hoja_de_viaje(void* arg)
{
	int comienzo = (int) arg;
	int i=0;
	int cantidad = list_size(entrenador->hoja_de_viaje);
	int estoy_muerto = 0;

	if(comienzo == NULL) {i =0;log_info(info_entrenador, "COMIENZO DEL RECORRIDO!");}
	else {i = comienzo; log_info(info_entrenador, "NUEVO INTENTO del recorrido de la HOJA DE VIAJE");}

	while(i<cantidad && !estoy_muerto)
	{
		entrenador_busca_mapa(i);
		conectar_a_mapa(entrenador->mapa_actual);
		enviar_mensaje_a_mapa(entrenador->mapa_actual,OTORGAR_SIMBOLO_ENTRENADOR, entrenador->simbolo);
		estoy_muerto=entrenador_cumpli_objetivos_del_mapa(i);
		entrenador_resetea_ubicacion();
		i++;
	}

	switch(estoy_muerto)
	{
		case(MUERTO):
		{
			log_info(info_entrenador, "FIN POR MUERTE");
			mapa_destruite(entrenador->mapa_actual);
			entrenador_finalizo_muriendo();
		};break;
		case(EXITO):
		{
			log_info(info_entrenador, "FIN DEL RECORRIDO!");
			entrenador_borra_pokemons();
			entrenador_borra_medallas();
		};break;
	}
}

void entrenador_busca_mapa(int index)
{
	char *nombre_mapa = list_get(entrenador->hoja_de_viaje, index);
	entrenador->mapa_actual = mapa_create(nombre_mapa, entrenador->ruta_pokedex, entrenador);

}

/*--------------------------------------------LOGICA DE CUMPLIR LOS OBJETIVOS DE UN MAPA---------------------------------*/
int entrenador_cumpli_objetivos_del_mapa(int index)
{
	//INICIO log
	char *mensaje = string_new();
	string_trim(&mensaje);
	string_append(&mensaje, "COMIENZO AVENTURA EN MAPA ");
	string_append(&mensaje, entrenador->mapa_actual->nombre);
	log_info(info_entrenador,mensaje);
	free(mensaje);
	//FIN log

	int ii=0;
	int cantidad_objetivos = list_size(entrenador->mapa_actual->objetivos);
	int estoy_muerto = 0;
	while(ii<cantidad_objetivos  && !estoy_muerto)
	{
		entrenador_pedi_ubicacion_pokenest(ii);
		estoy_muerto = entrenador_cumpli_objetivo(ii);
		free(entrenador->pokenest);
		ii++;
	}

	if(!estoy_muerto)
	{
		entrenador_termina_en_el_mapa();
		mapa_destruite(entrenador->mapa_actual);
		return 0;
	}
	else
	{
		if(me_quedan_vidas())
		{
			bajarvida(259);
			log_info(info_entrenador, "Volvere a empezar en el mismo mapa");
			int resultado = entrenador_volve_a_empezar_en_este_mapa(index);
			return resultado;
		}
		else
		{
			log_info(info_entrenador, "No tengo mas vidas disponibles para continuar en el mapa.");
			close(entrenador->mapa_actual->server);
			return 1;
		}
	}
}

int entrenador_volve_a_empezar_en_este_mapa(int indexxx)
{
	close(entrenador->mapa_actual->server);
	entrenador_borra_pokemons();
	list_destroy_and_destroy_elements(entrenador->mapa_actual->pokemons_capturados,mapa_element_destroyer);
	entrenador->mapa_actual->pokemons_capturados = list_create();

	//log_info(info_entrenador, "VOY A VOLVER A CONECTARME AL MAPA");
	//entrenador_busca_mapa(indexxx);
	usleep(1500*1000);
	conectar_a_mapa(entrenador->mapa_actual);
	enviar_mensaje_a_mapa(entrenador->mapa_actual,OTORGAR_SIMBOLO_ENTRENADOR, entrenador->simbolo);

	//log_info(info_entrenador, "COMIENZO DE NUEVO EN EL MAPA");
	entrenador_resetea_ubicacion();
	int resultado =entrenador_cumpli_objetivos_del_mapa(indexxx);
	return resultado;
}

void entrenador_borra_pokemos_del_mapa()
{
	int cantidad_pokemons = list_size(entrenador->mapa_actual->pokemons_capturados);
	int i=0;
	while(i<cantidad_pokemons)
	{
		char *element = list_get(entrenador->mapa_actual->pokemons_capturados,i);
		char *last_element = array_last_element(element);
		char *path = obtener_ruta_especifica(entrenador->directorio_de_bill,last_element,NULL);

		eliminar(path);
		free(last_element);
		free(path);
		i++;
	}
	list_clean(entrenador->mapa_actual->pokemons_capturados);
}

int me_quedan_vidas()
{
	if(entrenador->vidas > 0) return 1;
	else return 0;
}

void entrenador_termina_en_el_mapa()
{
	entrenador_copia_medalla_del_mapa();
	close(entrenador->mapa_actual->server);

	//INICIO log
	char *mensaje_fin = string_new();
	string_trim(&mensaje_fin);
	string_append(&mensaje_fin, "Termine en  ");
	string_append(&mensaje_fin, entrenador->mapa_actual->nombre);
	log_info(info_entrenador,mensaje_fin);
	free(mensaje_fin);
	//FIN log
}

void entrenador_espera_turno()
{
	char *turno =escuchar_mensaje_mapa(entrenador->mapa_actual, MAPA_ME_DA_TURNO);
	if(string_equals_ignore_case(turno, "tr;"))
	{
		free(turno);
		sem_post(&turno_entrenador);
	}
}

void entrenador_pedi_ubicacion_pokenest(int indice_objetivo)
{
	entrenador_espera_turno();
	sem_wait(&turno_entrenador);

	char *identificador_pokenest =list_get(entrenador->mapa_actual->objetivos,indice_objetivo);
	enviar_mensaje_a_mapa(entrenador->mapa_actual,SOLICITAR_COORDENADAS_POKENEST ,identificador_pokenest);

	char *mensaje_1 = string_new();
	string_append(&mensaje_1, "Quiero coordenadas de:  ");
	string_append(&mensaje_1,identificador_pokenest);
	log_info(info_entrenador,mensaje_1);
	free(mensaje_1);

	char *mapa_me_dice_que_me_envia_2 = escuchar_mensaje_mapa(entrenador->mapa_actual,MAPA_ME_AVISA_QUE_ME_VA_A_ENVIAR);
	int lo_que_me_da_mapa_2 =  mapa_me_dice(mapa_me_dice_que_me_envia_2);
	free(mapa_me_dice_que_me_envia_2);

	switch(lo_que_me_da_mapa_2)
	{
		case(MAPA_ME_DA_COORDENADAS_POKENEST):
		{
			char *coordenada = escuchar_mensaje_mapa(entrenador->mapa_actual, MAPA_ME_DA_COORDENADAS_POKENEST);
			t_ubicacion *ubicacion_pokenest = desarmar_coordenada(coordenada);
			free(coordenada);
			entrenador->pokenest = ubicacion_pokenest;
		};break;
	}

		log_info(info_entrenador,"LISTO! Tengo las coordenadas");
}

/*--------------------------------------------LOGICA DE CAMINAR HACIA POKENEST--------------------------------*/
int entrenador_cumpli_objetivo(int indice_obejtivo)
{
	//while(!entrenador_llego_a_destino() && me_quedan_vidas() //REVISAR ACA!!!!
	while(!entrenador_llego_a_destino() && !matan_al_entrenador)
	{
		entrenador_espera_turno();
		if(!matan_al_entrenador)
		{
			entrenador_ubicate_para_donde_caminar();
			loggear_posicion_actual();
			entrenador_informa_movimiento();
		}

	}
	if(!me_quedan_vidas() && matan_al_entrenador)
	{
		return MUERTO;
	}
	else
	{
		entrenador_espera_turno();
		int resultado=entrenador_captura_pokemon(indice_obejtivo);
		return resultado;
	}
}

void entrenador_informa_movimiento()
{
	char *posicion_actual = armar_coordenada(entrenador->ubicacion->x, entrenador->ubicacion->y, MAX_BYTES_COORDENADA);
	enviar_mensaje_a_mapa(entrenador->mapa_actual,REPORTAR_MOVIMIENTO, posicion_actual);
	free(posicion_actual);
}

/*--------------------------------------------LOGICA DE ATRAPAR POKEMON-------------------------------------*/
int entrenador_captura_pokemon(int indice_objetivo)
{
	//INICIO log
	char *mensaje = string_new();
	string_append(&mensaje, "Llegue! Voy a intentar capturar pokemon");
	log_info(info_entrenador,mensaje);
	free(mensaje);
	//FIN log

	enviar_mensaje_a_mapa(entrenador->mapa_actual,SOLICITAR_CAPTURA_POKEMON,NULL);

	char *hora_inicio_bloqueado = temporal_get_string_time();

	if(!me_quedan_vidas() && matan_al_entrenador)
	{
		char *hora_fin_desbloqueado = temporal_get_string_time();
		entrenador_registra_tiempo_bloqueo(hora_inicio_bloqueado, hora_fin_desbloqueado);
		return MUERTO;
	}
	else
	{
		int me_preguntan_boludeces = 1;
		int respuesta_mapa;
		while(me_preguntan_boludeces)
		{
			char *solicitud = escuchar_mensaje_mapa(entrenador->mapa_actual, MAPA_ME_AVISA_QUE_ME_VA_A_ENVIAR);
			respuesta_mapa = mapa_me_dice(solicitud);
			free(solicitud);
			if(respuesta_mapa == MAPA_ME_PREGUNTA_SI_ESTOY)
			{
				enviar_mensaje_a_mapa(entrenador->mapa_actual,SIGO_VIVO,NULL);
			}
			else
			{
				me_preguntan_boludeces = 0;
			}
		}
		switch(respuesta_mapa)
		{
			case(MAPA_ME_DA_POKEMON):
			{
				entrenador_recibi_y_copia_pokemon();
				char *hora_fin_desbloqueado = temporal_get_string_time();
				entrenador_registra_tiempo_bloqueo(hora_inicio_bloqueado, hora_fin_desbloqueado);
				return EXITO;
			};break;
			case(MAPA_DESCONECTADO):
			{
				if(matan_al_entrenador)
				{
					char *hora_fin_desbloqueado = temporal_get_string_time();
					entrenador_registra_tiempo_bloqueo(hora_inicio_bloqueado, hora_fin_desbloqueado);
					return MUERTO;
				}
			};break;
			case(MAPA_ME_AVISA_DEADLOCK):
			{
				int mapa_me_da_pokemon = 0;
				int gano_deadlocks = 1;
				while(gano_deadlocks && !mapa_me_da_pokemon)
				{
					entrenador->cantidad_deadlocks = entrenador->cantidad_deadlocks +1;
					int resultado_deadlock = entrenador_trata_deadlock();
					if(resultado_deadlock == MUERTO)	{gano_deadlocks=0; }
					else
					{
						char* mapa_me_dice_que_me_envia = escuchar_mensaje_mapa(entrenador->mapa_actual,MAPA_ME_AVISA_QUE_ME_VA_A_ENVIAR);
						int resultado = mapa_me_dice(mapa_me_dice_que_me_envia);
						free(mapa_me_dice_que_me_envia);
						if(resultado == MAPA_ME_DA_POKEMON)
						{
							entrenador_recibi_y_copia_pokemon();
							char *hora_fin_desbloqueado = temporal_get_string_time();
							entrenador_registra_tiempo_bloqueo(hora_inicio_bloqueado, hora_fin_desbloqueado);
							mapa_me_da_pokemon = 1;
						}
						else
						{
							gano_deadlocks = 1;
							mapa_me_da_pokemon = 0;
						}
					}
				}
				if(mapa_me_da_pokemon == 1)
				{
					return EXITO;
				}
				else
				{
					char *hora_fin_desbloqueado = temporal_get_string_time();
					entrenador_registra_tiempo_bloqueo(hora_inicio_bloqueado, hora_fin_desbloqueado);
					return MUERTO;
				}
			};break;
		}
	}
}

void entrenador_recibi_y_copia_pokemon()
{

		char *pokemon = escuchar_mensaje_mapa(entrenador->mapa_actual, MAPA_ME_DA_POKEMON);

		//INICIO log
		char *mensaje = string_new();
		string_append(&mensaje, entrenador->mapa_actual->nombre);
		string_append(&mensaje, " me dice que el pokemon esta en: ");
		string_append(&mensaje, pokemon);
		log_info(info_entrenador,mensaje);
		free(mensaje);
		//FIN log

		copiar(pokemon,entrenador->directorio_de_bill);
		list_add(entrenador->mapa_actual->pokemons_capturados, pokemon);

		//INICIO log
		char *mensaje_2 = string_new();
		string_trim(&mensaje_2);
		string_append(&mensaje_2, "Pokemon copiado y capturado!");
		log_info(info_entrenador,mensaje_2);
		free(mensaje_2);
		//FIN log

}

void entrenador_registra_tiempo_bloqueo(char *hora_inicio, char *hora_fin)
{
	int tiempo = diferencia_de_tiempos(hora_inicio, hora_fin);
	entrenador->tiempo_bloqueado_pokenest = entrenador->tiempo_bloqueado_pokenest + tiempo;
}

/*------------------------------------------LOGICA DE CAER EN DEADLOCK--------------------------------------------*/
int entrenador_trata_deadlock()
{
	log_info(info_entrenador,"Uy! Batalla pokemon!");
	entrenador_otorga_mejor_pokemon_a_mapa(entrenador->mapa_actual);
	log_info(info_entrenador,"Voy a escuchar si gane o perdi");
	char* respuesta = escuchar_mensaje_mapa(entrenador->mapa_actual, MAPA_ME_AVISA_QUE_ME_VA_A_ENVIAR);
	int caso_respuesta = mapa_me_dice(respuesta);
	free(respuesta);
	switch(caso_respuesta)
	{
		case(MAPA_ME_DICE_QUE_GANE):
		{
			log_info(info_entrenador,"GANE! Seguire esperando por el pokemon solicitado");
			return EXITO;
		}break;
		case(MAPA_ME_DICE_QUE_PERDI):
		{
			log_info(info_entrenador,"Perdi :( ");
			entrenador->muertes = entrenador->muertes +1;
			return MUERTO;
		}break;
	}
}

void entrenador_otorga_mejor_pokemon_a_mapa(t_mapa *mapa)
{
	t_pokemon *mejor_pokemon = entrenador_busca_mejor_pokemon();

	char* mensaje = string_new();
	string_append(&mensaje,"Mi pokemon elegido es: ");
	string_append(&mensaje,mejor_pokemon->species);
	log_info(info_entrenador,mensaje);
	free(mensaje);

	char *mejor_pokemon_serialized = armar_mejor_pokemon_string(mejor_pokemon);

	enviar_mensaje_a_mapa(mapa,ENTREGAR_MEJOR_POKEMON, mejor_pokemon_serialized);
	free(mejor_pokemon_serialized);
	pokemon_destroy(mejor_pokemon);
}



/*------------------------------------------LOGICA DE TERMINAR EN EL MAPA----------------------------------------*/
void entrenador_avisa_que_terminaste_en_este_mapa()
{
	enviar_mensaje_a_mapa(entrenador->mapa_actual, REPORTAR_FIN_OBJETIVOS, NULL);
	char *esperar_confirmacion_de_medalla = escuchar_mensaje_mapa(entrenador->mapa_actual,MAPA_ME_AVISA_QUE_ME_VA_A_ENVIAR);
	if(string_equals_ignore_case(esperar_confirmacion_de_medalla, "mr;"))
	{
		char *medalla_del_mapa = escuchar_mensaje_mapa(entrenador->mapa_actual, MAPA_ME_DA_MEDALLA);
		char *ruta_medallas = obtener_ruta_especifica(entrenador->directorio_de_bill, "medallas", NULL);
		string_append(&ruta_medallas,"/");
		copiar(medalla_del_mapa, ruta_medallas);
		free(ruta_medallas);
	}
	else { perror("No se puede interpretar mensaje"); }
}

void entrenador_copia_medalla_del_mapa()
{
	char *ruta_medalla_origen = obtener_ruta_especifica(entrenador->ruta_pokedex, "Mapas", entrenador->mapa_actual->nombre);
	char *aux = string_new();
	string_append(&aux,"medalla-");
	string_append(&aux,entrenador->mapa_actual->nombre);
	string_append(&aux,".jpg");
	char* ruta_medalla_origen_full = obtener_ruta_especifica(ruta_medalla_origen,aux,NULL);


	char *ruta_medallas_destino = obtener_ruta_especifica(entrenador->ruta_pokedex, "Entrenadores", entrenador->nombre);
	char* ruta_medallas_destino_full = obtener_ruta_especifica(ruta_medallas_destino,"medallas",NULL);
	string_append(&ruta_medallas_destino_full,"/");

	copiar(ruta_medalla_origen_full,ruta_medallas_destino_full);

	free(aux);
	free(ruta_medalla_origen);
	free(ruta_medallas_destino);
	free(ruta_medalla_origen_full);
	free(ruta_medallas_destino_full);
}

/*------------------------------------------BUSCAR MEJOR POKEMON-------------------------------------------------*/
t_pokemon* entrenador_busca_mejor_pokemon()
{
	log_info(info_entrenador,"Voy a buscar mi mejor pokemon");
	t_list *pokemons_names = nombre_de_archivos_del_directorio(entrenador->directorio_de_bill);
	t_list *pokemos = recuperar_pokemons(pokemons_names);

	list_destroy_and_destroy_elements(pokemons_names,destroy_path);

	list_sort(pokemos, order_mejor_pokemon);
	t_pokemon* mejor_pok = malloc (sizeof(t_pokemon));
	t_pokemon* mejor_pok_lista = list_get(pokemos,0);

	mejor_pok->species = string_new();
	string_append(&mejor_pok->species,mejor_pok_lista->species);

	mejor_pok->level = mejor_pok_lista->level;
	mejor_pok->second_type = mejor_pok_lista->second_type;
	mejor_pok->type = mejor_pok_lista->type;

	list_destroy_and_destroy_elements(pokemos,pokemon_destroy);
	return mejor_pok;
}

int order_mejor_pokemon(void* arg1, void* arg2)
{
	t_pokemon* pokemon_1 = (t_pokemon*) arg1;
	t_pokemon* pokemon_2 = (t_pokemon*) arg2;

	if(pokemon_1->level> pokemon_2->level)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void destroy_path(void *arg)
{
	char *path = (char*) arg;
	free(path);
}

void pokemon_destroy(void *arg)
{
	t_pokemon *pokemon = (t_pokemon*) arg;
	free(pokemon->species);
	free(pokemon);
}

t_list* recuperar_pokemons(t_list *lista_nombres_pokemons)
{
	int size = list_size(lista_nombres_pokemons);
	t_list *list_pokemons = list_create();
	t_pkmn_factory *factory = create_pkmn_factory();

	int i;
	for(i=0; i<size; i++)
	{
		char *name_file = list_get(lista_nombres_pokemons, i);
		t_pokemon *new_pokemon = recuperar_pokemon(factory,name_file);
		list_add(list_pokemons, new_pokemon);
	}
	destroy_pkmn_factory(factory);
	return list_pokemons;
}

t_pokemon* recuperar_pokemon(t_pkmn_factory *factory,char *nombre_file_pokemon)
{
	char *aux = obtener_ruta_especifica(entrenador->directorio_de_bill, nombre_file_pokemon,NULL);
	t_config *config_aux = config_create(aux);

	int nivel = config_get_int_value(config_aux,"Nivel");

	char* nombre_pokemon_sin_modificar = string_duplicate(nombre_file_pokemon);
	char* specie_pokemon = adaptar_nombre_pokemon(nombre_pokemon_sin_modificar);

	free(nombre_pokemon_sin_modificar);

	t_pokemon *new_pokemon = create_pokemon(factory,specie_pokemon,nivel);

	config_destroy(config_aux);
	free(aux);
	return new_pokemon;
}

char* adaptar_nombre_pokemon(char* nombre_sucio)
{
		//log_info(info_entrenador,nombre_sucio);


		/*char* aux = string_substring(nombre_sucio,0,string_length(nombre_sucio)-3);
		//string_to_upper(aux);
		//log_info(info_entrenador,aux);

		char* new_pokemon_adapter = malloc(sizeof(char)*(string_length(aux)+1));

		memcpy(new_pokemon_adapter, aux,1);
		string_to_lower(aux);

		memcpy(new_pokemon_adapter+1, aux+1, string_length(aux)-1);

		new_pokemon_adapter[string_length(aux)] = '\0';*/


		char* aux = string_substring(nombre_sucio,0,string_length(nombre_sucio)-7);
		aux[0]= toupper(aux[0]);

		//free(aux);
		//return new_pokemon_adapter;
		return aux;
}

/*------------------------------------------AUX --> LS -----------------------------------------------------------*/
t_list* nombre_de_archivos_del_directorio(char *ruta)
{

	/* Variables */
		DIR *dirp;
		struct dirent *direntp;

	/* Abrimos el directorio */
		dirp = opendir(ruta);
		if (dirp == NULL){
			printf("Error: No se puede abrir el directorio\n");
			exit(1);
		}
		t_list *lista = list_create();
		while ((direntp = readdir(dirp)) != NULL) {
			 if(!string_equals_ignore_case(direntp->d_name, ".")){
				  if(!string_equals_ignore_case(direntp->d_name, "..")){
					  if(!string_equals_ignore_case(direntp->d_name, "metadata") && string_ends_with(direntp->d_name,".dat"))
					  {
						  char* aux = string_new();
						  string_append(&aux,direntp->d_name);
						  list_add(lista, aux);
					  	}
				  }
			 }
		}
			closedir(dirp);
		 	 return lista;

	}

void loggear_posicion_actual()
{
	char* mensaje = string_new();
	char* x = string_itoa(entrenador->ubicacion->x);
	char* y = string_itoa(entrenador->ubicacion->y);

	string_append(&mensaje,"Me movi a: (");
	string_append(&mensaje,x);
	string_append(&mensaje,";");
	string_append(&mensaje,y);
	string_append(&mensaje,")");

	log_info(info_entrenador,mensaje);
	free(mensaje);
	free(x);
	free(y);
}
