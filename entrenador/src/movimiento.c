/*
 * movimiento.c
 *
 *  Created on: 27/9/2016
 *      Author: utnso
 */
#include "movimiento.h"

/*--------------------------------------------LOGICA DEL MOVIMIENTO DEL ENTRENADOR---------------------------------*/

void entrenador_ubicate_para_donde_caminar()
{
	switch(diferencia_de_posiciones(entrenador->ubicacion, entrenador->pokenest))
	{
		case(AUMENTAx_AUMENTAy): entrenador_camina_hacia_destino(AUMENTAR,AUMENTAR) ;break;
		case(AUMENTAx_DISMINUIy): entrenador_camina_hacia_destino(AUMENTAR,DISMINUIR);break;
		case(DISMINUIx_AUMENTAy): entrenador_camina_hacia_destino(DISMINUIR,AUMENTAR);break;
		case(DISMINUIx_DISMINUIy): entrenador_camina_hacia_destino(DISMINUIR,DISMINUIR);break;
	}
}

int diferencia_de_posiciones(t_ubicacion *posicion_actual, t_ubicacion *posicion_final)
{
	if((posicion_final->x - posicion_actual->x )>0)
	{
		if((posicion_final->y - posicion_actual->y) >0)
		{
			return AUMENTAx_AUMENTAy;
		}
		return AUMENTAx_DISMINUIy;
	}
	else
	{
		if((posicion_final->y - posicion_actual->y) >0)
		{
			return DISMINUIx_AUMENTAy;
		}
		return DISMINUIx_DISMINUIy;
	}
}

void entrenador_camina_hacia_destino(int orientacion_x, int orientacion_y)
{
	if(entrenador_llego_a_posicion_y())
	{
		entrenador_movete_en_eje(EJE_X,orientacion_x);

	}
	else
	{
		if(entrenador_llego_a_posicion_x())
		{
			entrenador_movete_en_eje(EJE_Y,orientacion_y);
		}
		else
		{
			entrenador_movete_alternado(orientacion_x,orientacion_y);
		}
	}

}

int entrenador_llego_a_destino()
{
	return ubicacion_coincide(entrenador->ubicacion, entrenador->pokenest);
}

int entrenador_llego_a_posicion_y()
{
	if(entrenador->pokenest->y == entrenador->ubicacion->y)
	{
		return 1;
	}
	else { return 0; }
}

int entrenador_llego_a_posicion_x()
{
	if(entrenador->pokenest->x == entrenador->ubicacion->x)
		{
			return 1;
		}
		else { return 0; }
}

int ubicacion_coincide(t_ubicacion *ubicacion1,t_ubicacion *ubicacion2)
{
	if((ubicacion1->x == ubicacion2->x) && (ubicacion1->y == ubicacion2->y))
	{
		return 1;
	}
	else {return 0; }
}

void entrenador_movete_en_eje(int eje, int orientacion)
{
	switch(eje)
	{
		case(EJE_X):
		{
			if(orientacion == AUMENTAR)
			{
				entrenador->ubicacion->x++;
			}
			else { entrenador->ubicacion->x--; }
		} break;
		case(EJE_Y):
		{
			if(orientacion == AUMENTAR)
			{
				entrenador->ubicacion->y++;
			}
			else { entrenador->ubicacion->y--; }
		}
	}

}

int paso_anterior_fue_en_x()
{
	if(entrenador->paso_anterior->x == 1)
	{
		return 1;
	}
	else { return 0; }
}

void entrenador_movete_alternado(int orientacion_x, int orientacion_y)
{
	if(paso_anterior_fue_en_x())
	{
		entrenador_movete_en_eje(EJE_Y,orientacion_y);
		entrenador->paso_anterior->x = 0;
		entrenador->paso_anterior->y = 1;
	}
	else
	{
		entrenador_movete_en_eje(EJE_X, orientacion_x);
		entrenador->paso_anterior->x = 1;
		entrenador->paso_anterior->y = 0;
	}
}

void entrenador_resetea_ubicacion()
{
	free(entrenador->ubicacion);
	entrenador->ubicacion = ubicacion_create(0,0);
	free(entrenador->paso_anterior);
	entrenador->paso_anterior = ubicacion_create(0,0);
	//free(entrenador->pokenest);
}
