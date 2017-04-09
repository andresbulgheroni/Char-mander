/*
 * comunicacion.h
 *
 *  Created on: 12/9/2016
 *      Author: utnso
 */

#ifndef MAPA_MAPA_COMMONS_COMUNICACION_H_
#define MAPA_MAPA_COMMONS_COMUNICACION_H_
#include "map-commons.h"
#include "basic-structs.h"
#include "socket/server.h"
#include "pthread.h"
#include "so-commons/process.h"
#include "pkmn/factory.h"

enum RESPUESTA_DEL_ENTRENADOR
{
	ENTRENADOR_ESTA_BUSCANDO_COORDENADAS_POKENEST = 11,
	ENTRENADOR_QUIERE_MOVERSE = 12,
	ENTRENADOR_QUIERE_CAPTURAR_POKEMON = 13,
	ENTRENADOR_FINALIZO_OBJETIVOS = 14,
	ENTRENADOR_OTORGA_SU_SIMBOLO = 15,
	ENTRENADOR_DESCONECTADO = 16,
	SOLICITUD_DEL_ENTRENADOR = 17,
	ENTRENADOR_SIGUE_VIVO = 18
} ;

enum ENVIOS_AL_ENTRENADOR
{
	OTORGAR_TURNO = 1,
	OTORGAR_COORDENADAS_POKENEST = 2,
	OTORGAR_MEDALLA_DEL_MAPA = 3,
	OTORGAR_POKEMON = 4,
	AVISAR_BLOQUEO_A_ENTRENADOR = 5,
	AVISAR_DESBLOQUEO_A_ENTRENADOR =6,
	AVISAR_DEADLOCK = 7,
	AVISAR_QUE_GANO = 8,
	AVISAR_QUE_PERDIO = 9,
	PREGUNTAR_SI_SIGUE_AHI = 10
};

#define MAX_BYTES_COORDENADA 5
#define MAX_BYTES_TOTAL_A_ENVIAR 100
#define SERVER_DESCONECTADO 0

extern t_mapa *mapa;
extern t_mapa *mapa;
extern sem_t semaforo_hay_algun_entrenador_listo;
extern pthread_mutex_t mutex_manipular_cola_nuevos;

typedef struct
{
	int socket_cliente;
	int recibir_mensaje_especifico;
	sem_t *semaforo_solicitudes;
	sem_t *semaforo_especificos;
}t_control_entrenador;


void* ejecutar_servidor(void *argumento);
void conexion_create(int *conexion);
void* atender_cliente(void* argumento);
void agregar_proceso_a_lista(int *socket_cliente, sem_t *semaforo_finalizacion);


/*-------------------------------------------DECODIFICACION DE RESPUESTAS------------------------------------------------*/
int tratar_respuesta(char* respuesta_del_entrenador, t_entrenador *entrenador);

void enviar_mensaje_a_entrenador(t_entrenador *entrenador, int header, char *payload);

char* escuchar_mensaje_entrenador(t_entrenador *entrenador, int header);

t_posicion* desarmar_coordenada(char *coordenada);

void dar_pokemon_a_entrenador(t_entrenador *entrenador,char *ruta_pokemon);

void otorgar_ruta_medalla_a_entrenador(int entrenador, char *rutaMedalla);

void* escuchar_mejor_pokemon(int socket_entrenador);

#endif /* MAPA_MAPA_COMMONS_COMUNICACION_H_ */
