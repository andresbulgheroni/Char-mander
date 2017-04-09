/*
 * movimiento.h
 *
 *  Created on: 27/9/2016
 *      Author: utnso
 */

#ifndef MOVIMIENTO_H_
#define MOVIMIENTO_H_

#include "basic-structs.h"
#include "entrenador-commons.h"

enum ubicacion_coordenada
{
	AUMENTAx_DISMINUIy,
	DISMINUIx_AUMENTAy,
	AUMENTAx_AUMENTAy,
	DISMINUIx_DISMINUIy
};

enum ejes
{
	EJE_X,
	EJE_Y
};

enum movimiento
{
	AUMENTAR,
	DISMINUIR
};

extern t_entrenador *entrenador;

int entrenador_llego_a_destino();
void entrenador_camina_hacia_destino();
int ubicacion_coincide(t_ubicacion *ubicacion1,t_ubicacion *ubicacion2);
int paso_anterior_fue_en_x();
int entrenador_llego_a_destino();
int entrenador_llego_a_posicion_y();
int entrenador_llego_a_posicion_x();
void entrenador_movete_alternado(int orientacion_x, int orientacion_y);
void entrenador_ubicate_para_donde_caminar();
void entrenador_movete_en_eje(int eje, int orientacion);
void entrenador_camina_hacia_destino(int orientacion_x, int orientacion_y);
int diferencia_de_posiciones(t_ubicacion *posicion_actual, t_ubicacion *posicion_final);
void entrenador_resetea_ubicacion();
#endif /* MOVIMIENTO_H_ */
