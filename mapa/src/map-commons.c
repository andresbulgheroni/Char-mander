/*
 * map-commons.c
 *
 *  Created on: 4/9/2016
 *      Author: utnso
 */
#include "map-commons.h"

/*------------------------------------------VARIABLES GLOBALES----------------------------------------------------*/
t_mapa *mapa;

sem_t semaforo_entrenadores_listos;
sem_t semaforo_servidor;
sem_t semaforo_terminacion;
sem_t semaforo_cola_entrenadores_sin_objetivos;
sem_t semaforo_esperar_por_entrenador_listo;
pthread_mutex_t mutex_manipular_cola_listos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_manipular_cola_nuevos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_manipular_cola_bloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_manipular_cola_finalizados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_entrenadores_sin_objetivos = PTHREAD_MUTEX_INITIALIZER;

int servidor_debe_terminar = 0;
int algoritmo_cambio = 0;
t_info_algoritmo *nuevo_algoritmo;
int hay_jugadores_online;
int hay_jugadores;

/*------------------------------------------EXECUTE----------------------------------------------------------------*/
void ejecutar_mapa(char *nombre, char *rutaPokedex)
{
	iniciar_semaforos();
	iniciar_logs(nombre);
	iniciar_seniales_mapa();

	mapa = mapa_create(nombre, rutaPokedex);
	log_info(informe_mapa, "Mapa creado correctamente");

	if(PANTALLA_ESTA_ACTIVADA)
	{
		mapa_mostrate_en_pantalla();
	}
	hay_jugadores = 0;
	hay_jugadores_online = 0;
	planificador_create_segun_cual_seas();
	mapa_ejecuta_deadlock();
	//encolacion_entrenadores_iniciada=NO_INICIADO;
	mapa_hacete_visible_para_entrenadores();
}

void iniciar_seniales_mapa()
{
	signal(SIGUSR2, releer_data);
}

void releer_data(int n)
{
	log_info(informe_mapa, "SEÑAL DETECTADA!");
	t_info_algoritmo *algotirmo_viejo = mapa->info_algoritmo;

	config_destroy(mapa->configuracion);
	t_config *nueva_configuracion = configuracion_metadata_create(mapa->nombre,mapa->ruta_pokedex);
	mapa->configuracion = nueva_configuracion;

	mapa->tiempo_chequeo_deadlock = obtener_info_mapa_tiempo_deadlock(mapa->configuracion);
	mapa->batalla = obtener_info_mapa_batalla(mapa->configuracion);

	t_info_algoritmo *nuevo_algoritmo = obtener_info_mapa_algoritmo(nueva_configuracion);
	cambiar_algoritmo_si_es_necesario(algotirmo_viejo,nuevo_algoritmo);

	//config_destroy(nueva_configuracion);
}

void iniciar_semaforos()
{
	sem_init(&semaforo_entrenadores_listos,1,0);
	sem_init(&semaforo_cola_entrenadores_sin_objetivos,1,0);
	sem_init(&semaforo_servidor,1,0);
	sem_init(&semaforo_esperar_por_entrenador_listo,1,0);
}

void iniciar_logs(char *nombre)
{
	char *nombre_log = string_new();
	string_append(&nombre_log, "Log ");
	string_append(&nombre_log,nombre);
	informe_mapa =log_create(nombre_log, "Proceso mapa", 0, LOG_LEVEL_INFO);
	free(nombre_log);
}

void cambiar_algoritmo_si_es_necesario(t_info_algoritmo *viejo, t_info_algoritmo *nuevo)
{
	if(!string_equals_ignore_case(viejo->algoritmo, nuevo->algoritmo))
	{
		log_info(informe_mapa, "Se cambia el algoritmo de planificacion");
		algoritmo_cambio = 1;
		nuevo_algoritmo = nuevo;
	}
	else
	{
		mapa->info_algoritmo->quamtum = nuevo->quamtum;
		mapa->info_algoritmo->retardo = nuevo->retardo;
		destroy_info_algoritmo(nuevo);
	}
}

void cambiar_algoritmo()
{
	destroy_info_algoritmo(mapa->info_algoritmo);
	mapa->info_algoritmo = nuevo_algoritmo;
	if(mapa_decime_si_planificador_es(PLANIFICADOR_RR))
	{
		hay_jugadores_online = hay_jugadores;
	}
	else
	{
		hay_jugadores = hay_jugadores_online;
	}
	planificador_create_segun_cual_seas();
	algoritmo_cambio=0;
}

/*--------------------------------------------PRINCIPALES----------------------------------------------------------*/
t_posicion* mapa_dame_coordenadas_de_pokenest(char* identificador_pokemon)
{
	t_pokeNest *pokeNest_buscado = mapa_buscame_pokenest(identificador_pokemon);
	return pokeNest_buscado->posicion;
}

t_pokeNest* mapa_buscame_pokenest(char *identificador_pokemon)
{
	t_pokeNest *pokeNest_buscado = dictionary_get(mapa->pokeNests, identificador_pokemon);
	return pokeNest_buscado;
}

char* mapa_dame_medalla()
{
	char *ruta_medalla = obtener_ruta_especifica(mapa->ruta_pokedex, "Mapas",mapa->nombre);
	char* ruta_medalla_2 = obtener_ruta_especifica(ruta_medalla,"medalla-",mapa->nombre);
	char* ruta_medalla_full = obtener_ruta_especifica(ruta_medalla_2,".jpg",NULL);
	free(ruta_medalla);
	free(ruta_medalla_2);
	return ruta_medalla_full;
}

int mapa_decime_si_hay_pokemones_en_pokenest(char *id_pokenest)
{
	t_pokeNest *pokenest = mapa_buscame_pokenest(id_pokenest);
	return (pokenest->cantidad_pokemones_disponibles>0);
}

char* mapa_dame_pokemon_de_pokenest(char *id_pokenest)
{
	t_pokeNest *pokenest = mapa_buscame_pokenest(id_pokenest);
	char *ruta_de_algun_pokemon = queue_pop(pokenest->pokemones);
	pokenest->cantidad_pokemones_disponibles--;
	return ruta_de_algun_pokemon;
}

int mapa_decime_si_entrenador_esta_bloqueado(t_entrenador *entrenador)
{
	if(entrenador->estado == BLOQUEADO)
	{
		return 1;
	}
	else {return 0;}
}

int mapa_decime_si_entrenador_esta_abortado(t_entrenador *entrenador)
{
	if(entrenador->objetivo_cumplido==ABORTADO)
	{
		return 1;
	}
	else {return 0;}
}

int mapa_decime_si_entrenador_estaba_bloqueado(t_entrenador *entrenador)
{
	if(entrenador->estado_anterior == 0)
		{
			return 1;
		}
		else {return 0;}
}

int mapa_decime_si_entrenador_esta_listo_pero_estaba_bloqueado(t_entrenador *entrenador)
{
	return (entrenador->estado==LISTO && mapa_decime_si_entrenador_estaba_bloqueado(entrenador));
}

int mapa_decime_si_entrenador_esta_ejecutando_pero_estaba_bloqueado(t_entrenador *entrenador)
{
	return (entrenador->estado == EXECUTE && entrenador->estado_anterior == BLOQUEADO);
}

void mapa_cambiale_estado_a_entrenador(t_entrenador *entrenador, int estado_entrante, int estado_saliente)
{
	entrenador->estado = estado_entrante;
	entrenador->estado_anterior = estado_saliente;
}

void mapa_devolve_pokemon_a_pokenest(char *ruta_pokemon)
{
	char* id_pokenest = obtener_id_ponekest(ruta_pokemon);
	t_pokeNest *pokenest = mapa_buscame_pokenest(id_pokenest);
	queue_push(pokenest->pokemones, ruta_pokemon);
	pokenest->cantidad_pokemones_disponibles++;

	free(id_pokenest);

	if(PANTALLA_ESTA_ACTIVADA)
	{
		char id = (char)(pokenest->identificador[0]);
		BorrarItem(mapa->items_para_mostrar_en_pantalla, id);
		CrearCaja(mapa->items_para_mostrar_en_pantalla,id,pokenest->posicion->x,pokenest->posicion->y,pokenest->cantidad_pokemones_disponibles);
		nivel_gui_dibujar(mapa->items_para_mostrar_en_pantalla,mapa->nombre);
	}
}

int mapa_decime_si_entrenador_finalizo_su_objetivo(int socket_entrenador)
{
	char *key = string_itoa(socket_entrenador);
	return(dictionary_has_key(mapa->entrenadores->lista_entrenadores_finalizados, key));
	free(key);
}

void mapa_actualiza_distancia_del_entrenador(t_entrenador *entrenador)
{
	t_posicion *posicion_final = mapa_dame_coordenadas_de_pokenest(entrenador->pokenest_objetivo);
	int distancia_en_x;
	int distancia_en_y;
	if((posicion_final->x) - (entrenador->posicion_actual->x) > 0)
	{
		distancia_en_x = (posicion_final->x) - (entrenador->posicion_actual->x);
	}
	else
	{
		distancia_en_x = (entrenador->posicion_actual->x) - (posicion_final->x);
	}
	if((posicion_final->y) - (entrenador->posicion_actual->y) > 0)
	{
		distancia_en_y = (posicion_final->y) - (entrenador->posicion_actual->y);
	}
	else
	{
		distancia_en_y = (entrenador->posicion_actual->y)-(posicion_final->y);
	}
	entrenador->distancia_hasta_objetivo = distancia_en_x+distancia_en_y;
}

/*--------------------------------------------------- FUNCIONES PARA GRAFICAR--------------------------------------------*/
 void mapa_mostrate_en_pantalla()
{

	mapa->items_para_mostrar_en_pantalla= list_create();

	int filas, columnas;
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&filas, &columnas);
	mapa_agrega_pokenest_a_items_para_pantalla();
	nivel_gui_dibujar(mapa->items_para_mostrar_en_pantalla, mapa->nombre);

	log_info(informe_mapa, "Mapa comienza a mostrar interfaz gráfica");

}

void mapa_elimina_entrenador_de_pantalla(t_entrenador *entrenador)
{
	if(PANTALLA_ESTA_ACTIVADA)
	{
		char id = (char)entrenador->simbolo_identificador[0];
		BorrarItem(mapa->items_para_mostrar_en_pantalla, id);
		nivel_gui_dibujar(mapa->items_para_mostrar_en_pantalla, mapa->nombre);

		//INICIO LOG
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "Se borra de pantalla a entrenador identificado por ");
		string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
		log_info(informe_mapa, mensaje_A_loggear);
		free(mensaje_A_loggear);
		//FIN LOG
	}
}

void mapa_actualiza_pokemones_disponibles_de_pokenest(char *id_pokenest)
 {
	if(PANTALLA_ESTA_ACTIVADA)
	{
		t_pokeNest *pokenest = mapa_buscame_pokenest(id_pokenest);
		char id = (char)(pokenest->identificador[0]);
		restarRecurso(mapa->items_para_mostrar_en_pantalla, id);
	}
 }

void mapa_agrega_pokenest_a_items_para_pantalla()
{
	if(PANTALLA_ESTA_ACTIVADA)
	{
		dictionary_iterator(mapa->pokeNests, closure);
		log_info(informe_mapa, "Se agregan pokenest a interfaz gráfica");
	}
}

void closure(char *identificador, void *data )
{
	t_pokeNest *pokenest=(t_pokeNest*) data;
	char id = (char)(pokenest->identificador[0]);
	CrearCaja(mapa->items_para_mostrar_en_pantalla,id,pokenest->posicion->x,pokenest->posicion->y,pokenest->cantidad_pokemones_disponibles);
}

void mapa_mostra_nuevo_entrenador_en_pantalla(t_entrenador *entrenador)
{
	if(PANTALLA_ESTA_ACTIVADA)
	{
		char id = (char)entrenador->simbolo_identificador[0];
		int x = entrenador->posicion_actual->x;
		int y= entrenador->posicion_actual->y;
		CrearPersonaje(mapa->items_para_mostrar_en_pantalla, id,x,y);
		nivel_gui_dibujar(mapa->items_para_mostrar_en_pantalla, mapa->nombre);

		//INICIO LOG
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "Se grafica a nuevo entrenador identificado con el simbolo ");
		string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
		log_info(informe_mapa, mensaje_A_loggear);
		free(mensaje_A_loggear);
		//FIN LOG
	}
}

void mapa_mostra_actualizacion_de_entrenador(t_entrenador *entrenador)
{
	if(PANTALLA_ESTA_ACTIVADA)
	{
		char id = (char)entrenador->simbolo_identificador[0];
		int x = entrenador->posicion_actual->x;
		int y= entrenador->posicion_actual->y;
		MoverPersonaje(mapa->items_para_mostrar_en_pantalla, id,x,y);
		nivel_gui_dibujar(mapa->items_para_mostrar_en_pantalla, mapa->nombre);

		//INICIO LOG
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "Se grafica movimiento de entrenador identificado con el simbolo ");
		string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
		log_info(informe_mapa, mensaje_A_loggear);
		free(mensaje_A_loggear);
		//FIN LOG
	}
}

void mapa_borra_entrenador_de_pantalla(t_entrenador *entrenador)
{
	if(PANTALLA_ESTA_ACTIVADA)
	{
		char id = *(entrenador->simbolo_identificador);
		BorrarItem(mapa->items_para_mostrar_en_pantalla, id);
		nivel_gui_dibujar(mapa->items_para_mostrar_en_pantalla, mapa->nombre);

		//INICIO LOG
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "Se borra de interfaz gráfica a entrenador identificado con el simbolo ");
		string_append(&mensaje_A_loggear, entrenador->simbolo_identificador);
		log_info(informe_mapa, mensaje_A_loggear);
		free(mensaje_A_loggear);
		//FIN LOG
	}
}

/*------------------------------ FUNCIONES PARA MANIPULACION DEL PLANIFICADOR--------------------------------------------*/
void planificador_create_segun_cual_seas()
{
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(string_equals_ignore_case(mapa->info_algoritmo->algoritmo, "RR"))
	{
		pthread_create(&thread,NULL,ejecutar_planificador_rr, NULL);
		pthread_attr_destroy(&attr);
		log_info(informe_mapa, "Se crea hilo de planificador RR");
	}
	else{
		pthread_create(&thread,NULL,ejecutar_planificador_srdf,NULL);
		pthread_attr_destroy(&attr);
		log_info(informe_mapa, "Se crea hilo de planificador SRDF");
	}
}

int mapa_decime_si_planificador_es(int planificador)
{
	switch(planificador)
	{
		case(PLANIFICADOR_RR):return(string_equals_ignore_case(mapa->info_algoritmo->algoritmo,"RR"));break;
		case(PLANIFICADOR_SRDF):return(string_equals_ignore_case(mapa->info_algoritmo->algoritmo,"SRDF"));break;
	}
}

/*----------------------- FUNCIONES PARA MANIPULACION DE ENTRENADORES (MEDIANTE SOCKETS)-------------------------------*/
void mapa_hacete_visible_para_entrenadores()
{
	/*thread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t hilo_servidor;
	pthread_create(&hilo_servidor,&attr,ejecutar_servidor,(void*)mapa->info_socket);

	log_info(informe_mapa, "Se comienza a ejecutar hilo de escucha");
	pthread_attr_destroy(&attr);*/
	ejecutar_servidor((void*)mapa->info_socket);
}

/*----------------------------------------FUNCIONES PARA DEADLOCK--------------------------------------------------------*/
void mapa_ejecuta_deadlock()
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t hilo_servidor;
	pthread_create(&hilo_servidor,&attr,ejecutar_deadlock,NULL);

	log_info(informe_mapa, "Se comienza a ejecutar hilo de deadlock");
	pthread_attr_destroy(&attr);
}


