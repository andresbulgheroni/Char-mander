/*
 * entrenador-commons.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef ENTRENADOR_COMMONS_H_
#define ENTRENADOR_COMMONS_H_
#include "so-commons/log.h"

#include "comunication.h"
#include "basic-structs.h"
#include "time.h"
#include "semaphore.h"
#include "pthread.h"
#include "movimiento.h"
#include <dirent.h>
#include "factory.h"

t_log *info_entrenador;
t_entrenador *entrenador;

enum time
{
	INICIO,
	FIN
};

enum
{
	MUERTO=1,
	EXITO =0,
	REINTENTAR=2
};

enum
{
	GANE_DEADLOKC,
	PERDI_DEADLOCK,
	ME_DIERON_POKEMON
};

char *hora_de_inicio;
char *hora_de_fin;

void mostrar_por_pantalla_resultados();

//void iniciar_semaforos();
void iniciar_log(char *nombre_del_entrenador);
void entrenador_registra_hora(int rango);
void entrenador_comenza_aventura();
void entrenador_recorre_este_mapa();
void matar_entrenador();

void ejecutar_entrenador(char *nombre_entrenador, char *ruta_pokedex);
void entrenador_comenza_a_explorar();

void entrenador_recorre_hoja_de_viaje(void* arg);
int entrenador_cumpli_objetivos_del_mapa(int index);
int entrenador_volve_a_empezar_en_este_mapa(int indexxx);

void entrenador_termina_en_el_mapa();
int entrenador_cumpli_objetivo(int indice_objetivo);
void entrenador_busca_mapa(int index);
void entrenador_copia_medalla_del_mapa();

int entrenador_espera_mientras_te_dice_pavadas();
void entrenador_espera_turno();
void entrenador_espera_a_que_mapa_te_bloquee();
void entrenador_espera_a_que_mapa_te_desbloquee();

void entrenador_iniciar_seniales();
void subirvida(int n);
void bajarvida(int n);
void matar_entrenador(int n);

void entrenador_finalizo_muriendo();
void tratar_respuesta();


void entrenador_reinicia_hoja_de_viaje();
void entrenador_borra_medallas();
void entrenador_borra_pokemons();
void entrenador_morite();

void entrenador_pedi_ubicacion_pokenest(int indice_objetivo);
void entrenador_recibi_y_copia_pokemon();
void entrenador_registra_tiempo_bloqueo(char *hora_inicio, char *hora_fin);

void entrenador_informa_movimiento();
void entrenador_avisa_que_terminaste_en_este_mapa();
int entrenador_captura_pokemon(int indice_objetivo);


int entrenador_trata_deadlock();
int me_quedan_vidas();
void bajarvida();
void entrenador_borra_pokemos_del_mapa();

/*------------------------------------------BUSCAR MEJOR POKEMON-------------------------------------------------*/
void entrenador_otorga_mejor_pokemon_a_mapa(t_mapa *mapa);
t_pokemon* entrenador_busca_mejor_pokemon();
t_list* nombre_de_archivos_del_directorio(char *ruta);
t_list* recuperar_pokemons(t_list *lista_nombres_pokemons);
t_pokemon* recuperar_pokemon(t_pkmn_factory *factory,char *nombre_file_pokemon);
int order_mejor_pokemon(void* arg1, void* arg2);
char* adaptar_nombre_pokemon(char* nombre_sucio);
void pokemon_destroy(void *arg);

void destroy_path(void *arg);
void loggear_posicion_actual();
#endif /* ENTRENADOR_COMMONS_H_ */
