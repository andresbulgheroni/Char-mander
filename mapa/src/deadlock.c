/*
 * deadlock.c
 *
 *  Created on: 3/11/2016
 *      Author: utnso
 */
#include "deadlock.h"
extern char** vector_auxiliar_identificadores_pokenest;
extern t_mapa* mapa;
t_dictionary* procesos;
t_dictionary* procesos_identificados_por_indice;
int cantidad_columnas_actuales = 0;
int cantidad_columnas_ocupadas = 0;
int cantidad_procesos = 0;
int indice_posicion_proceso = 0;
int cantidad_solicitudes = 0;

pthread_mutex_t mutex_operaciones_deadlock = PTHREAD_MUTEX_INITIALIZER;
//sem_t semaforo_esperar_para_revisar;

int* vector_procesos_sin_recursos_asignados;
int* vector_T;

t_log* logger;

/*----------------------------------------------INICIALIZACION----------------------------------------------------------*/
t_estructuras_deteccion* deadlock_inicializate()
{
	t_estructuras_deteccion *new_deadlock = malloc(sizeof(t_estructuras_deteccion));
	new_deadlock->vector_recursos_totales = inicializar_recursos_totales();
	new_deadlock->vector_recursos_disponibles = inicializar_recursos_disponibles();
	new_deadlock->matriz_asignacion=inicializar_matriz_asignacion();
	new_deadlock->matriz_solicitud = inicializar_matriz_solicitud();
	procesos = dictionary_create();
	procesos_identificados_por_indice = dictionary_create();

	vector_procesos_sin_recursos_asignados = malloc(sizeof(int)+1);
	vector_procesos_sin_recursos_asignados[0] = -1;

	cantidad_columnas_actuales = 1;


	return new_deadlock;
}

void limpiar_vector_marcados()
{
	free(vector_procesos_sin_recursos_asignados);
	vector_procesos_sin_recursos_asignados = malloc(sizeof(int)+1);
	vector_procesos_sin_recursos_asignados[0] = -1;
}

void loggear_informacion(int caso)
{
	switch(caso)
	{
	case(LOG_INICIALIZACION):
	{
		log_info(logger, "Se inicializa correctamente el proceso de Deadlock");
	};break;
	case(LOG_PROCESOS_MARCADOS):
	{
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "VECTOR MARCADOS:  ");

		int tamanio = tamanio_vector(vector_procesos_sin_recursos_asignados);
		if(tamanio == 0)
		{
			string_append(&mensaje_A_loggear,"actualmente vacío." );
		}
		else
		{
			int i;
			for(i=0;i<tamanio;i++)
			{
				char* valor= string_itoa(vector_procesos_sin_recursos_asignados[i]);
				string_append(&mensaje_A_loggear,valor );
				string_append(&mensaje_A_loggear, " ");
				free(valor);
			}
		}
		log_info(logger, mensaje_A_loggear);
		free(mensaje_A_loggear);
	}break;
	case(LOG_VECTOR_T):
	{
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "VECTOR T:  ");

		int tamanio = tamanio_vector(vector_T);
		if(tamanio == 0)
		{
			string_append(&mensaje_A_loggear,"actualmente vacío." );
		}
		else
		{
			int i;
			for(i=0;i<tamanio;i++)
			{
				char* valor= string_itoa(vector_T[i]);
				string_append(&mensaje_A_loggear,valor );
				string_append(&mensaje_A_loggear, " ");
				free(valor);
			}
		}
		log_info(logger, mensaje_A_loggear);
		free(mensaje_A_loggear);
	}break;
	case(LOG_RECURSOS_TOTALES):
	{
		char *mensaje_A_loggear = string_new();
				string_append(&mensaje_A_loggear, "VECTOR RECURSOS TOTALES:  ");

				int tamanio = tamanio_vector(deadlock->vector_recursos_totales);
				if(tamanio == 0)
				{
					string_append(&mensaje_A_loggear,"actualmente vacío." );
				}
				else
				{
					int i;
					for(i=0;i<tamanio;i++)
					{
						char* valor= string_itoa(deadlock->vector_recursos_totales[i]);
						string_append(&mensaje_A_loggear,valor );
						string_append(&mensaje_A_loggear, " ");
						free(valor);
					}
				}
				log_info(logger, mensaje_A_loggear);
				free(mensaje_A_loggear);
	};break;
	case(LOG_RECURSOS_DISPONIBLES):
	{
		char *mensaje_A_loggear = string_new();
				string_append(&mensaje_A_loggear, "VECTOR RECURSOS DISPONIBLES:  ");

				int tamanio = tamanio_vector(deadlock->vector_recursos_disponibles);
				if(tamanio == 0)
				{
					string_append(&mensaje_A_loggear,"actualmente vacío." );
				}
				else
				{
					int i;
					for(i=0;i<tamanio;i++)
					{
						char* valor= string_itoa(deadlock->vector_recursos_disponibles[i]);
						string_append(&mensaje_A_loggear,valor );
						string_append(&mensaje_A_loggear, " ");
						free(valor);
					}
				}
				log_info(logger, mensaje_A_loggear);
				free(mensaje_A_loggear);
	};break;
	case(LOG_VECTOR_AUXILIAR_POKENEST):
	{
		char *mensaje_A_loggear = string_new();
		string_append(&mensaje_A_loggear, "VECTOR AUXILIAR POKENEST:  ");

		int tamanio = tamanio_vector_ids(vector_auxiliar_identificadores_pokenest);
		if(tamanio == 0)
			{
			string_append(&mensaje_A_loggear,"actualmente vacío." );
			}
		else
			{
				int i;
			for(i=0;i<tamanio;i++)
			{
				string_append(&mensaje_A_loggear,vector_auxiliar_identificadores_pokenest[i] );
				string_append(&mensaje_A_loggear, " ");
			}
			}
			log_info(logger, mensaje_A_loggear);
			free(mensaje_A_loggear);
	};break;
	case(LOG_DEADLOCK):
	{
		log_info(logger,"DEADLOCK DETECTADO");
	};break;
	}
}

void loggear_vector_asignacion_proceso(char* id_proceso ,int proceso)
{
	char* mensaje_a_log = string_new();
	string_append(&mensaje_a_log,"VECTOR ASIGNACIÓN PROCESO ");
	string_append(&mensaje_a_log, id_proceso);
	string_append(&mensaje_a_log," ID ");
	char* proceso_string = string_itoa(proceso);
	string_append(&mensaje_a_log, proceso_string);
	free(proceso_string);
	string_append(&mensaje_a_log," : ");
	int tamanio = dictionary_size(mapa->pokeNests);
	int i;
	for(i=0;i<tamanio;i++)
	{
		char* valor= string_itoa(deadlock->matriz_asignacion[i][proceso]);
		string_append(&mensaje_a_log,valor );
		string_append(&mensaje_a_log, " ");
		free(valor);
	}

	log_info(logger, mensaje_a_log);
	free(mensaje_a_log);
}

void loggear_vector_solicitudes_proceso(char* id_proceso ,int proceso)
{
	char* mensaje_a_log = string_new();
		string_append(&mensaje_a_log,"VECTOR SOLICITUDES PROCESO ");
		string_append(&mensaje_a_log, id_proceso);
		string_append(&mensaje_a_log," ID ");
		char* proceso_String = string_itoa(proceso);
		string_append(&mensaje_a_log, proceso_String);
		free(proceso_String);

		string_append(&mensaje_a_log," : ");
		int tamanio = dictionary_size(mapa->pokeNests);
		int i;
		for(i=0;i<tamanio;i++)
		{
			char* valor= string_itoa(deadlock->matriz_solicitud[i][proceso]);
			string_append(&mensaje_a_log,valor );
			string_append(&mensaje_a_log, " ");
			free(valor);
		}

		log_info(logger, mensaje_a_log);
		free(mensaje_a_log);
}

int* inicializar_recursos_totales()
{
	int cantidad_pokenest = dictionary_size(mapa->pokeNests);
	int* new_vector = malloc((cantidad_pokenest+1) * sizeof(int));

	int i;
	for(i=0;i<cantidad_pokenest;i++)
	{
		char* id = vector_auxiliar_identificadores_pokenest[i];
		t_pokeNest *pokenest = dictionary_get(mapa->pokeNests,id);
		new_vector[i] = pokenest->cantidad_pokemones_disponibles;
	}
	new_vector[cantidad_pokenest] = -1;
	return new_vector;
}

int* inicializar_recursos_disponibles()
{
	int cantidad_pokenest = dictionary_size(mapa->pokeNests);
	int* new_vector = malloc((cantidad_pokenest+1) * sizeof(int));

	int i;
	for(i=0;i<cantidad_pokenest;i++)
	{
		t_pokeNest *pokenest = dictionary_get(mapa->pokeNests,vector_auxiliar_identificadores_pokenest[i]);
		new_vector[i] = pokenest->cantidad_pokemones_disponibles;
	}
	new_vector[cantidad_pokenest] = -1;
	return new_vector;
}

int** inicializar_matriz_asignacion()
{
	int cantidad_pokenest = dictionary_size(mapa->pokeNests);
	int** new_matriz = malloc(cantidad_pokenest*sizeof(int));

	/* ASIGNO ESPACIO PARA UNA SOLA COLUMNA A TODAS LAS FILAS */
	int i;
	for(i=0;i<cantidad_pokenest;i++)
	{
		new_matriz[i] = malloc(sizeof(int));
		new_matriz[i][0] = 0;
	}
	return new_matriz;
}

int** inicializar_matriz_solicitud()
{
	int cantidad_pokenest = dictionary_size(mapa->pokeNests);
	int** new_matriz = malloc(cantidad_pokenest*sizeof(int));
	/* ASIGNO ESPACIO PARA UNA SOLA COLUMNA A TODAS LAS FILAS */
	int i;
	for(i=0;i<cantidad_pokenest;i++)
	{
		new_matriz[i] = malloc(sizeof(int));
		new_matriz[i][0] = 0;
	}
	return new_matriz;
}

void limpiar_filas(int matriz)
{
	switch(matriz)
	{
		case(MATRIZ_ASIGNACION):
		{
			int i;
			int cantidad_pokenest = dictionary_size(mapa->pokeNests);

			for(i=0;i<cantidad_pokenest;i++)
			{
				deadlock->matriz_asignacion[i][0]=0;
			}

		};break;
		case(MATRIZ_SOLICITUD):
		{
			int i;
			int cantidad_pokenest = dictionary_size(mapa->pokeNests);

			for(i=0;i<cantidad_pokenest;i++)
			{
				deadlock->matriz_solicitud[i][0]=0;
			}
		};break;
	}
}

void iniciar_vectorT()
{
	int cantidad_pokenest = dictionary_size(mapa->pokeNests);
	vector_T= malloc((cantidad_pokenest +1)* sizeof(int));
	memcpy(vector_T,deadlock->vector_recursos_disponibles,cantidad_pokenest * sizeof(int));
	vector_T[cantidad_pokenest] = -1;
}

void actualizar_vector_recursos_disponibles()
{
	free(deadlock->vector_recursos_disponibles);
	deadlock->vector_recursos_disponibles = inicializar_recursos_disponibles();
}

/*----------------------------------------------MANIPULACION ESTRUCTURAS-------------------------------------------------*/
int identificar_numero_fila(char* id_recurso_solicitado)
{
	int i=0;
	int encontrado = 0;
	int cantidad_pokenest = dictionary_size(mapa->pokeNests);
	while(i<cantidad_pokenest && !encontrado)
	{
		char* id = vector_auxiliar_identificadores_pokenest[i];
		if(string_equals_ignore_case(id,id_recurso_solicitado))
		{
			encontrado=1;
		}
		else
		{
			i++;
		}
	}

	return i;
}

void deadlock_agregar_nuevo_proceso_a_matrices(char* id_proceso)
{
	t_proceso *new_proceso = malloc(sizeof(t_proceso));
	new_proceso->id = string_new();
	string_append(&new_proceso->id,id_proceso);
	new_proceso->posicion = indice_posicion_proceso;

	cantidad_procesos++;

	char* mensaje_a_log = string_new();
	string_append(&mensaje_a_log,"SE AGREGA A LAS MATRICES AL PROCESO IDENTIFICADO POR: ");
	string_append(&mensaje_a_log, id_proceso);
	string_append(&mensaje_a_log," Y SU NUMERO DE ID ES: ");
	char* proceso_string = string_itoa(indice_posicion_proceso);
	string_append(&mensaje_a_log,proceso_string );
	free(proceso_string);
	log_info(logger,mensaje_a_log);
	free(mensaje_a_log);

	dictionary_put(procesos, id_proceso, new_proceso);
	char* indice_string = string_itoa(indice_posicion_proceso);
	dictionary_put(procesos_identificados_por_indice,indice_string,new_proceso);

	indice_posicion_proceso++;

	if(cantidad_columnas_actuales == cantidad_columnas_ocupadas)
	{
		pthread_mutex_lock(&mutex_operaciones_deadlock);
		cantidad_columnas_actuales++;
		cantidad_columnas_ocupadas++;
		asignar_nueva_columna_a_matriz(MATRIZ_ASIGNACION);
		asignar_nueva_columna_a_matriz(MATRIZ_SOLICITUD);
		pthread_mutex_unlock(&mutex_operaciones_deadlock);
	}
	else
	{
		cantidad_columnas_ocupadas++;
		// YA HAY ESPACIO PARA LA COLUMNA Y SE CORRESPONDERÁ CON EL INDICE EN LA LISTA
	}

}

void asignar_nueva_columna_a_matriz(int matriz)
{
	/* ASIGNO ESPACIO PARA UNA SOLA COLUMNA A TODAS LAS FILAS A PARTIR DE LA ULTIMA COLUMNA*/
	switch(matriz)
	{
		case(MATRIZ_ASIGNACION):
		{
			int i;
			int cantidad_pokenest = dictionary_size(mapa->pokeNests);
			for(i=0;i<cantidad_pokenest;i++)
			{
				deadlock->matriz_asignacion[i] = realloc(deadlock->matriz_asignacion[i],cantidad_columnas_actuales*sizeof(int));
				deadlock->matriz_asignacion[i][cantidad_columnas_actuales-1] = 0;
			}
		};break;
		case(MATRIZ_SOLICITUD):
		{
			int i;
			int cantidad_pokenest = dictionary_size(mapa->pokeNests);
			for(i=0;i<cantidad_pokenest;i++)
			{
				deadlock->matriz_solicitud[i] = realloc(deadlock->matriz_solicitud[i],cantidad_columnas_actuales*sizeof(int));
				deadlock->matriz_solicitud[i][cantidad_columnas_actuales-1]=0;
			}
		};break;
	}
}

void deadlock_actualizar_matriz(char* id_proceso, char* id_recurso, int matriz,int tipo_actualizacion)
{
	pthread_mutex_lock(&mutex_operaciones_deadlock);
	int numero_fila_del_recurso = identificar_numero_fila(id_recurso);
	t_proceso* proceso = dictionary_get(procesos,id_proceso);

	if(matriz == MATRIZ_ASIGNACION)
	{
		char* mensaje_a_log = string_new();
			string_append(&mensaje_a_log,"Proceso idedentificado por ");
			string_append(&mensaje_a_log, id_proceso);
			string_append(&mensaje_a_log," y por ID ");
			char* proceso_string = string_itoa(proceso->posicion);
			string_append(&mensaje_a_log, proceso_string);
			free(proceso_string);
			if(tipo_actualizacion==OTORGAR_RECURSO)
			{
				string_append(&mensaje_a_log,"  se le dá recurso: ");
				string_append(&mensaje_a_log, id_recurso);
			}
			else
			{
				string_append(&mensaje_a_log," devuelve recurso: ");
				string_append(&mensaje_a_log, id_recurso);
			}
			log_info(logger,mensaje_a_log);
			free(mensaje_a_log);
	}
	else
	{
		if(tipo_actualizacion==OTORGAR_RECURSO)
		{
			char* mensaje_a_log = string_new();
			string_append(&mensaje_a_log,"Proceso idedentificado por ");
			string_append(&mensaje_a_log, id_proceso);
			string_append(&mensaje_a_log," y por ID ");
			char* proceso_string = string_itoa(proceso->posicion);
			string_append(&mensaje_a_log, proceso_string);
			free(proceso_string);
			string_append(&mensaje_a_log,"  está solicitando: ");
			string_append(&mensaje_a_log, id_recurso);
			log_info(logger,mensaje_a_log);
			free(mensaje_a_log);
		}
	}


	switch(matriz)
	{
		case(MATRIZ_ASIGNACION):
		{
			switch(tipo_actualizacion)
			{
				case(OTORGAR_RECURSO):
				{
					deadlock->matriz_asignacion[numero_fila_del_recurso][proceso->posicion] = deadlock->matriz_asignacion[numero_fila_del_recurso][proceso->posicion] +1;
					deadlock->vector_recursos_disponibles[numero_fila_del_recurso] = deadlock->vector_recursos_disponibles[numero_fila_del_recurso] -1;
					loggear_vector_asignacion_proceso(id_proceso,proceso->posicion);
					loggear_informacion(LOG_RECURSOS_DISPONIBLES);
				};break;
				case(QUITAR_RECURSO):
				{
					deadlock->matriz_asignacion[numero_fila_del_recurso][proceso->posicion] = deadlock->matriz_asignacion[numero_fila_del_recurso][proceso->posicion] -1;
					deadlock->vector_recursos_disponibles[numero_fila_del_recurso] = deadlock->vector_recursos_disponibles[numero_fila_del_recurso] +1;
					loggear_vector_asignacion_proceso(id_proceso,proceso->posicion);
					loggear_informacion(LOG_RECURSOS_DISPONIBLES);
				};break;
			}
		};break;

		case(MATRIZ_SOLICITUD):
		{
			switch(tipo_actualizacion)
			{
				case(OTORGAR_RECURSO):
				{
					deadlock->matriz_solicitud[numero_fila_del_recurso][proceso->posicion] = deadlock->matriz_solicitud[numero_fila_del_recurso][proceso->posicion] +1;
					cantidad_solicitudes++;
					loggear_vector_solicitudes_proceso(id_proceso,proceso->posicion);
					//log_info(logger, "SE RESERVA RECURSO DE MATRIZ DE SOLICITUD");
				};break;
				case(QUITAR_RECURSO):
				{
					deadlock->matriz_solicitud[numero_fila_del_recurso][proceso->posicion] = deadlock->matriz_solicitud[numero_fila_del_recurso][proceso->posicion] -1;
					cantidad_solicitudes--;
					loggear_vector_solicitudes_proceso(id_proceso,proceso->posicion);
					//log_info(logger, "SE LIBERA RECURSO DE MATRIZ DE SOLICITUD");
				};break;
			}
		};break;
	}
	pthread_mutex_unlock(&mutex_operaciones_deadlock);
}

void deadlock_elimina_proceso_de_matrices(char* id_proceso)
{
	pthread_mutex_lock(&mutex_operaciones_deadlock);
	cantidad_procesos--;

	char* mensaje_a_log = string_new();
	string_append(&mensaje_a_log,"SE ELIMINA DE MATRICES AL PROCESO IDENTIFICADO POR: ");
	string_append(&mensaje_a_log, id_proceso);
	log_info(logger,mensaje_a_log);
	free(mensaje_a_log);

	/* POR CUESTIONES DE SIMPLICIDAD, NO SE ELIMINARA LA COLUMNA, SINO QUE SOLO SE MARCARÁ TODAS SUS CELDAS CON UN -1 */
	t_proceso *proceso=dictionary_remove(procesos,id_proceso);

	int cantidad_pokenest = dictionary_size(mapa->pokeNests);

	/* SE SUMAN LOS RECURSOS QUE POSEÍA AL VECTOR DE DISPONIBLES */
	int recorrido_recursos_asignados;
	for(recorrido_recursos_asignados=0;recorrido_recursos_asignados<cantidad_pokenest;recorrido_recursos_asignados++)
	{
		deadlock->vector_recursos_disponibles[recorrido_recursos_asignados] = deadlock->matriz_asignacion[recorrido_recursos_asignados][proceso->posicion] + deadlock->vector_recursos_disponibles[recorrido_recursos_asignados];

	}
	loggear_informacion(LOG_RECURSOS_DISPONIBLES);

	/* SE LIMPIA LA MATRIZ DE SOLICITUD */
	int i;
	for(i=0;i<cantidad_pokenest;i++)
	{
	deadlock->matriz_solicitud[i][proceso->posicion] = -1;
	}

	/* SE LIMPIA LA MATRIZ DE ASIGNACION */
	int i2;
	for(i2=0;i2<cantidad_pokenest;i2++)
	{
		deadlock->matriz_asignacion[i2][proceso->posicion] = -1;
	}

	free(proceso->id);
	free(proceso);
	pthread_mutex_unlock(&mutex_operaciones_deadlock);
}

void quitar_proceso_de_vectores(int numero_proceso)
{
	if(proceso_esta_marcado(numero_proceso))
	{

	}
	if(proceso_esta_en_vectorT(numero_proceso))
	{

	}
}

t_list* obtener_las_victimas()
{
	t_list *new_list = list_create();

	int i;
	for(i=0;i<cantidad_columnas_ocupadas;i++)
	{
		if(!proceso_esta_borrado(i) && !proceso_esta_marcado(i) && proceso_tiene_solicitudes(i))
		{
			char* index_string = string_itoa(i);
			t_proceso *proceso = (t_proceso*) dictionary_get(procesos_identificados_por_indice,index_string);
			free(index_string);
			char* id_proceso = string_new();
			string_append(&id_proceso, proceso->id);
			list_add(new_list,id_proceso);
		}
	}

	return new_list;
}

/*----------------------------------------------EJECUCION------------------------------------------------------------*/
void ejecutar_deadlock(void* arg)
{
	char* nombre_log = string_new();
	string_append(&nombre_log,"Log deadlock ");
	string_append(&nombre_log, mapa->nombre);
	logger = log_create(nombre_log, "Deadlock",0, LOG_LEVEL_INFO);
	free(nombre_log);

	//sem_init(&semaforo_esperar_para_revisar,0,0);
	deadlock=deadlock_inicializate();
	loggear_informacion(LOG_INICIALIZACION);
	loggear_informacion(LOG_RECURSOS_TOTALES);
	loggear_informacion(LOG_RECURSOS_DISPONIBLES);
	loggear_informacion(LOG_PROCESOS_MARCADOS);
	deadlock_revisa();
}

void deadlock_revisa()
{
	while(1)
	{
		usleep(mapa->tiempo_chequeo_deadlock*1000);

		if(cantidad_solicitudes>=2)
		{
		pthread_mutex_lock(&mutex_operaciones_deadlock);
		log_info(logger, "Nueva iteración");
		limpiar_vector_marcados();

		marcar_procesos_que_no_tienen_recursos_asignados();
		pthread_mutex_unlock(&mutex_operaciones_deadlock);

		if(tamanio_vector(vector_procesos_sin_recursos_asignados)!= 0)
		{
			loggear_informacion(LOG_PROCESOS_MARCADOS);
		}
		iniciar_vectorT();
		loggear_informacion(LOG_VECTOR_T);
		int se_sigue_ejecutando = 1;

		while(se_sigue_ejecutando)
		{
			pthread_mutex_lock(&mutex_operaciones_deadlock);
			if(cantidad_columnas_ocupadas>0 && cantidad_procesos>0 && cantidad_procesos !=1)
			{
					int se_encontro_proceso = marcar_proceso_si_se_puede_satisfacer();

					pthread_mutex_unlock(&mutex_operaciones_deadlock);

					if(tamanio_vector(vector_procesos_sin_recursos_asignados)!= 0)
					{
						loggear_informacion(LOG_PROCESOS_MARCADOS);
					}
					if(!se_encontro_proceso)
					{
						loggear_informacion(LOG_DEADLOCK);
						int resultado = resolver_deadlock();
						if(resultado==FALSA_ALARMA)
						{
							//log_info(logger,"FALSA ALAMRA.");
							se_sigue_ejecutando = 0;
						}
						else
						{
							//log_info(logger,"Se termino de resolver DEADLOCK");
							se_sigue_ejecutando = 0;
						}

					}
					else
					{
						se_sigue_ejecutando = 0;
					}

			}
			else
			{
				se_sigue_ejecutando = 0;
				pthread_mutex_unlock(&mutex_operaciones_deadlock);
			}
		}
		free(vector_T);
	}
	}
}

void marcar_procesos_que_no_tienen_recursos_asignados()
{
	int i;
	/* SE RECORRE POR COLUMNA -> UNA COLUMNA = UN PROCESO */
	for(i=0;i<cantidad_columnas_ocupadas;i++)
	{
		if(!proceso_esta_borrado(i))
		{
			if(!proceso_tiene_recursos_asignados(i))
			{
				if(!proceso_esta_marcado(i))
				{
					if(proceso_tiene_solicitudes(i))
					{
						marcar_proceso(i);
					}
				}

			}
		}
	}
}

void marcar_proceso(int proceso)
{
	int tamanio_del_vector = tamanio_vector(vector_procesos_sin_recursos_asignados);
	vector_procesos_sin_recursos_asignados = realloc(vector_procesos_sin_recursos_asignados, (tamanio_del_vector+2)*sizeof(int));
	vector_procesos_sin_recursos_asignados[tamanio_del_vector] = proceso;
	vector_procesos_sin_recursos_asignados[tamanio_del_vector+1] = -1;
}

int marcar_proceso_si_se_puede_satisfacer()
{
	int i=0;
	int proceso_candidato_encontrado = 0;

	while(i<cantidad_columnas_ocupadas && !proceso_candidato_encontrado)
	{
		if(!proceso_esta_borrado(i) && !proceso_esta_marcado(i) && proceso_tiene_solicitudes(i))
		{
			if(proceso_puede_satisfacerce(i))
			{
				proceso_candidato_encontrado = 1;
			}
		}
		i++;
	}

	if(proceso_candidato_encontrado)
	{
		marcar_proceso(i-1);
		int *vector = recuperar_vector_proceso(i-1,MATRIZ_ASIGNACION);
		sumar_vector_a_vectorT(vector);
		free(vector);
		return 1;
	}
	else
	{
		return 0;
	}
}

int resolver_deadlock()
{
	t_list* victimas = obtener_las_victimas();
	if(list_size(victimas) <=1)
	{
		list_destroy(victimas);
		return FALSA_ALARMA;
	}
	else
	{
		resolver_deadlock_si_es_posible(victimas);
		//resolver_deadlock_pokemon(victimas);
		return EXITO;
	}
}

/*----------------------------------------------AUXILIARES-----------------------------------------------------------*/
int proceso_porId_tiene_solicitudes(char* id_proceso)
{
	t_proceso* proceso = dictionary_get(procesos, id_proceso);
	int resltado = proceso_tiene_solicitudes(proceso->posicion);
	return resltado;
}

int proceso_tiene_solicitudes(int numero_proceso)
{
	int i=0;
	int cantidad_elementos = dictionary_size(mapa->pokeNests);

	/* SE RECORRE POR COLUMNA -> UNA COLUMNA = UN PROCESO */
	int tiene_recursos = 0;
	while(i<cantidad_elementos && tiene_recursos<=0)
	{
		tiene_recursos = tiene_recursos + deadlock->matriz_solicitud[i][numero_proceso];
		i++;
	}
	if(tiene_recursos > 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int algun_proceso_tiene_solicitudes()
{
	int i=0;
	int alguno_tiene_solicitud = 0;
	while(i<cantidad_columnas_ocupadas && !alguno_tiene_solicitud)
	{
		alguno_tiene_solicitud = proceso_tiene_solicitudes(i);
		i++;
	}
	return alguno_tiene_solicitud;
}

int proceso_puede_satisfacerce(int numero_proceso)
{
	int* vector_del_proceso = recuperar_vector_proceso(numero_proceso,MATRIZ_SOLICITUD);

	int resultado = vector_es_menor_igual_a_vectorT(vector_del_proceso);
	free(vector_del_proceso);

	return resultado;
}

int* recuperar_vector_proceso(int num_proceso, int matriz)
{
	int cantidad_elementos = dictionary_size(mapa->pokeNests);
	int* aux = malloc(sizeof(int) * (cantidad_elementos+1));
	aux[cantidad_elementos] = -1;

	switch(matriz)
	{
		case(MATRIZ_ASIGNACION):
		{
			int i;
			for(i=0;i<cantidad_elementos;i++)
			{
				aux[i]= deadlock->matriz_asignacion[i][num_proceso];
			}
		};break;
		case(MATRIZ_SOLICITUD):
		{
			int i;
			for(i=0;i<cantidad_elementos;i++)
			{
				aux[i]= deadlock->matriz_solicitud[i][num_proceso];
			}
		};break;
	}

	return aux;

}

int proceso_esta_borrado(int numero_proceso)
{
	int i=0;
	int cantidad_elementos = dictionary_size(mapa->pokeNests);

	/* SE RECORRE POR COLUMNA -> UNA COLUMNA = UN PROCESO */
	int esta_borrado = 0;
	while(i<cantidad_elementos && esta_borrado>=0)
	{
		esta_borrado = deadlock->matriz_asignacion[i][numero_proceso];
		i++;
	}
	if(esta_borrado<0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int proceso_tiene_recursos_asignados(int numero_proceso)
{
	int i=0;
	int cantidad_elementos = dictionary_size(mapa->pokeNests);

	/* SE RECORRE POR COLUMNA -> UNA COLUMNA = UN PROCESO */
	int tiene_recursos = 0;
	while(i<cantidad_elementos && tiene_recursos<=0)
	{
		tiene_recursos = tiene_recursos + deadlock->matriz_asignacion[i][numero_proceso];
		i++;
	}
	if(tiene_recursos > 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int proceso_esta_marcado(int numero_proceso)
{
	int tamanio = tamanio_vector(vector_procesos_sin_recursos_asignados);
	int i=0;
	int encontrado = 0;

	while(i<tamanio && !encontrado)
	{
		if(vector_procesos_sin_recursos_asignados[i] == numero_proceso)
		{
			encontrado = 1;
		}
		i++;
	}
	return encontrado;
}

int proceso_esta_en_vectorT(int numero_proceso)
{
	int tamanio = tamanio_vector(vector_T);
	int i=0;
	int encontrado = 0;

	while(i<tamanio && !encontrado)
	{
		if(vector_T[i] == numero_proceso)
		{
				encontrado = 1;
		}
		i++;
	}
	return encontrado;
}

int tamanio_vector_ids(char** vector)
{
	int i=0;
	int elementos = 0;
	while(!string_equals_ignore_case(vector[i],"-1"))
	{
		elementos++;
		i++;
	}
	return elementos;
}

int tamanio_vector(int* vector)
{
	int i=0;
	int elementos = 0;
	while(vector[i] != -1)
	{
		elementos++;
		i++;
	}
	return elementos;
}

int vector_es_menor_igual_a_vectorT(int* vector)
{
	int tamanio_vector_t = tamanio_vector(vector_T);

	int i=0;
	int rompe_condicion = 0;

	while(i<tamanio_vector_t && !rompe_condicion)
	{
		int valor_1 = vector[i];
		int valor_2 = vector_T[i];
		if(valor_1>valor_2)
		{
			rompe_condicion = 1;
		}
		i++;
	}
	if(rompe_condicion==1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void sumar_vector_a_vectorT(int* vector)
{
	int size = tamanio_vector(vector_T);
	int i;
	for(i=0;i<size;i++)
	{
		vector_T[i] = vector_T[i] + vector[i];
	}
}

