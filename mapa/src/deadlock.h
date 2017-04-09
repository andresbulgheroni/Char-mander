/*
 * deadlock.h
 *
 *  Created on: 3/11/2016
 *      Author: utnso
 */

#ifndef DEADLOCK_H_
#define DEADLOCK_H_

#include "basic-structs.h"
#include "map-commons.h"
#include "deadlock-resolucion.h"
#include "so-commons/collections/dictionary.h"

#define FALSA_ALARMA 0
#define EXITO 1

enum
{
	LOG_INICIALIZACION,
	LOG_PROCESOS_MARCADOS,
	LOG_VECTOR_T,
	LOG_DEADLOCK,
	LOG_RECURSOS_DISPONIBLES,
	LOG_RECURSOS_TOTALES,
	LOG_VECTOR_AUXILIAR_POKENEST

};

typedef struct
{
	int posicion;
	char* id;
}t_proceso;

typedef struct
{
	int** matriz_asignacion;
	int** matriz_solicitud;
	int* vector_recursos_totales;
	int* vector_recursos_disponibles;
}t_estructuras_deteccion;

t_estructuras_deteccion* deadlock;

enum
{
	QUITAR_RECURSO,
	OTORGAR_RECURSO
};

enum
{
	MATRIZ_ASIGNACION,
	MATRIZ_SOLICITUD
};

/*		 ESTRUCTURA DE LAS MATRICES
 *
 *	EJEMPLO:
 *
 * 		E1	E2	E3
 * P1	0	0	0
 * P2	1	0	1
 * P3	1	2	3
 * PN	8	0	0
 *
 *	 -EL TAMAÑO DE LAS FILAS ES FIJO -> SON LOS RECURSOS -> POKENEST
 *	 -EL TAMAÑO DE LAS COLUMNAS ES VARIABLE -> SON LOS PROCESOS -> ENTRENADORES
 *
 *
 *	 	ESTRUCTURA DE LOS VECTORES
 *
 *	 P1	P2	P3	... PN
 *	 1	2	4		0
 *
 *	 -CADA COLUMNA ES UN RECURSO -> CANTIDAD POKEMONES POKENEST
 *	 -LAS POSICIONES DE LAS COLUMNAS SE CORRESPONDEN CON EL VECTOR AUXILIAR DE ID_POKENEST
 *
 *	 -LAS POSICIONES DE LAS COLUMNAS DE LOS VECTORES SE CORRESPONDEN CON LAS POSICIONES DE LAS FILAS DE LAS MATRICES
 */

/*----------------------------------------------INICIALIZACION--------------------------------------------------------*/
t_estructuras_deteccion* deadlock_inicializate();
int* inicializar_recursos_totales();
int* inicializar_recursos_disponibles();
int** inicializar_matriz_asignacion();
int** inicializar_matriz_solicitud();
void iniciar_vectorT();
void actualizar_vector_recursos_disponibles();
void limpiar_vector_marcados();

void loggear_informacion(int caso);
void loggear_vector_asignacion_proceso(char* id_proceso ,int proceso);
void loggear_vector_solicitudes_proceso(char* id_proceso ,int proceso);
/*----------------------------------------------MANIPULACION ESTRUCTURAS-------------------------------------------------*/
int identificar_numero_fila(char* id_recurso_solicitado);
void deadlock_actualizar_matriz(char* id_proceso, char* id_recurso, int matriz,int tipo_actualizacion);
void asignar_nueva_columna_a_matriz(int matriz);
void deadlock_agregar_nuevo_proceso_a_matrices(char* id_proceso);
void deadlock_elimina_proceso_de_matrices(char* id_proceso);
t_list* obtener_las_victimas();
/*----------------------------------------------EJECUCION-----------------------------------------------------------------*/
void ejecutar_deadlock(void *arg);
void deadlock_revisa();
void marcar_procesos_que_no_tienen_recursos_asignados();
int marcar_proceso_si_se_puede_satisfacer();
void marcar_proceso(int proceso);
int resolver_deadlock();
/*----------------------------------------------AUXILIARES--------------------------------------------------------------*/
int proceso_porId_tiene_solicitudes(char* id_proceso);
int algun_proceso_tiene_solicitudes();
int proceso_tiene_solicitudes(int numero_proceso);
int proceso_puede_satisfacerce(int numero_proceso);
int* recuperar_vector_proceso(int num_proceso, int matriz);
int proceso_tiene_recursos_asignados(int numero_proceso);
int proceso_esta_borrado(int numero_proceso);
int proceso_esta_marcado(int numero_proceso);
int proceso_esta_en_vectorT(int numero_proceso);
int tamanio_vector(int* vector);
void limpiar_filas(int matriz);
int vector_es_menor_igual_a_vectorT(int* vector);
void sumar_vector_a_vectorT(int* vector);

int tamanio_vector_ids(char** vector);
#endif /* DEADLOCK_H_ */
