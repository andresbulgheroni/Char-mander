/*
 * time.h
 *
 *  Created on: 14/9/2016
 *      Author: utnso
 */

#ifndef TIME_H_
#define TIME_H_
#include "so-commons/string.h"
#include "so-commons/temporal.h"
#include <stdlib.h>
#include "math.h"

long de_hora_a_ms(int hora);
long de_min_a_ms(int minutos);
long de_seg_a_ms(int segundos);
long hora(char* tiempo);
long minutos(char* tiempo);
long segundos(char* tiempo);
long milisegundos(char* tiempo);
long tiempo_a_milisegundos(char* tiempo);
long diferencia_de_tiempos(char* tiempoOrigen, char* tiempoFinal);

#endif /* TIME_H_ */
