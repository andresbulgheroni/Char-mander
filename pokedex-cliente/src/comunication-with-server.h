/*
 * comunication-with-server.h
 *
 *  Created on: 5/10/2016
 *      Author: utnso
 */

#ifndef COMUNICATION_WITH_SERVER_H_
#define COMUNICATION_WITH_SERVER_H_


#include "vars.h"
#include <fuse.h>
#include "socket/serializacion_pokedex_cliente_servidor.h"
#include "socket/cliente.h"
#include "so-commons/log.h"
#define MAX_BYTES_TO_ADVISES 1
#include <pthread.h>


t_log *log;

/*-------------------------------------------CREACION Y CONEXION CON SERVER----------------------------------------------*/
void iniciar_log();
void cliente_osada_create();
void cliente_osada_conectate();

/*-------------------------------------------CREACION-----------------------------------------------------------------*/
int cliente_pedi_crear_directorio(const char *path, mode_t modo_de_creacion);
int cliente_pedi_crear_archivo(const char *path, mode_t modo, dev_t permisos);
/*-------------------------------------------ATRIBUTOS-----------------------------------------------------------------*/
int cliente_pedi_atributos(const char *path, struct stat *buffer);
int cliente_pedi_listado(const char *path, void *buffer, fuse_fill_dir_t filler);
/*-------------------------------------------ELMINACION-----------------------------------------------------------------*/
int cliente_pedi_eliminar(int tipo,const char *path);
/*-------------------------------------------WRITE & READ---------------------------------------------------------------*/
int cliente_pedi_leer_archivo(const char *path, char *buf, size_t size, off_t offset);
int cliente_pedi_escribir_archivo(const char *path, const char *text, size_t size, off_t offset, struct fuse_file_info *f);
/*-------------------------------------------RENAME---------------------------------------------------------------*/
int cliente_pedi_renombra_archivo(const char *old_path, const char *new_path);
/*-------------------------------------------OPENS & CLOSER--------------------------------------------------------*/
int cliente_pedi_abrir(int tipo,const char *path, struct fuse_file_info *fi);
/*-------------------------------------------TRUNCATE----------------------------------------------------------------*/
int cliente_pedi_truncar(const char* path, off_t size);
/*-------------------------------------------TIMES---------------------------------------------------------------*/
int cliente_pedi_times(const char* path, const struct timespec ts[2]);

int escuchar_respuesta_comun(int socket_server);
int escuchar_y_modelar_atributos(struct stat *buffer);
void modelar_stat_buff(struct stat *buffer, int tipo, int size);

int escuchar_listado (void *buffer, fuse_fill_dir_t filler);
int modelar_listado(void *buffer, fuse_fill_dir_t filler);
void modelar_listado_2(char* data,void *buffer, fuse_fill_dir_t filler);
#endif /* COMUNICATION_WITH_SERVER_H_ */
