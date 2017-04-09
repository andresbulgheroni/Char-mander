/*
 * file_manipuling.h
 *
 *  Created on: 12/10/2016
 *      Author: utnso
 */

#ifndef FILE_MANIPULING_H_
#define FILE_MANIPULING_H_
#include "osada.h"
#include "osada_generales.h"
#include "ls_and_atributes.h"
/*------------------------------------------CREAR ARCHIVO---------------------------------------------------------------------*/
t_osada_file_free* osada_b_file_create(int tipo, char* path);
t_osada_file_free* osada_b_crear(int tipo, char* path);
void setear_nombre(char* nombre, osada_file* archivo);
int calcular_posicion_en_tabla_de_archivos_absoluta(int bloque, int posicion_dentro_del_bloque);
void asignar_bloque_inicial_si_es_necesario(osada_file *file, int tipo);
int osada_b_get_a_new_block_init();
void setear_bloque_padre(osada_file *file, char *path);
int osada_b_check_repeat_name(int tipo,char* path);

/*----------------------------------------------OBTENCION DE NUM BLOQUE ARCHIVO-----------------------------------------*/
/*
 * @NAME: t_list* osada_get_blocks_nums_of_this_file(osada_file *file, t_disco_osada *disco)
 * @DESC: Dado un archivo osada,y un disco, devuelve TODOS los bloques que lo componen, revisando la TABLA DE ASIGNACIONES.
 *
 * WARNING: DEVUELVE UNA LISTA, POR LO QUE LUEGO DE USARLA, LA MISMA DEBERÁ SER LIBERADA.
 */
t_list* osada_get_blocks_nums_of_this_file(osada_file *file, t_disco_osada *disco);

/*---------------------------------------------BUSQUEDA DE UN ARCHIVO DISPONIBLE Y VACIO---------------------------------*/
/*
 * STRUCT T_OSADA_FILE_FREE
 * @CAMPOS:
 * 		-Un archivo vacío y listo para ser manipulado
 * 		-El número de bloque RELATIVO dentro de la TABLA DE ARCHIVOS (servirá para poder IMPACTARLO en disco)
 * 		-El número de posición en el bloque.
 * 			-position_in_block = 0 si es el primer File dentro del bloque
 * 			-position_in_block = 1 si es el segundo File dentro del bloque
 *
 * 	*NOTA: RECUERDE QUE LA UNA ESTRUCTURA DE TIPO OSADA_FILE PESA 32 BYTES, Y COMO LOS BLOQUES SON DE 64 BYTES,
 * 			EN UN BLOQUE ENTRAN DOS ARCHIVOS. DE AQUÍ LA JUSTIFICACIÓN DEL CAMPO POSITION_IN_BLOCK
 */


/*
 * @NAME:t_osada_file_free*  osada_file_table_get_space_free(t_disco_osada *disco);
 * @DESC: Dado un disco osada, devuelve un puntero a una estructura T_OSADA_FILE_FREE
 *
 */
t_osada_file_free* osada_file_table_get_space_free(t_disco_osada *disco);

/*
 * @NAME:int verify_file_state(int state,osada_file *file);
 * @DESC: Dado un ESTADO posible para el archivo (DELETED, REGULAR, DIRECTORY) y un archivo,
 * 		devuelve 1 si el estado coincide, 0 caso default.
 */
int verify_file_state(int state,osada_file *file);


/*
 * @NAME:void osada_change_file_state(osada_file *file, osada_file_state new_state);
 * @DESC: Dado un ARCHIVO y un estado DISTINTO al actual (DELETED, REGULAR, DIRECTORY), lo setea en el archivo.
 *
 *  WARNING: LOS CAMBIOS NO SON IMPACTADOS. AL FINALIZAR DE MANIPULAR EL ARCHIVO POR COMPLETO, EL PROGRAMADOR DEBERÁ
 *  		LLAMAR A ALGUNA DE LAS FUNCIONES DE IMPACTO EN DISCO
 */
void osada_change_file_state(osada_file *file, osada_file_state new_state);

int osada_check_is_table_asig_is_full();
/*----------------------------------------------OBTENCION DE UN ARCHIVO ESPECIFICO---------------------------------------*/


/*
 * @NAME:void* osada_get_file_called(char *file_name, t_disco_osada *disco);
 * @DESC: Dado un POSIBLE nombre de archivo,y un disco osada, devuelve "NO_EXISTE" (string) si el nombre de archivo dado
 * 		no se corresponde con ninguno del disco, o el archivo en caso de que lo encuentre.
 *
 * 	WARNING: NOTESE QUE ESTA FUNCION SOLO COMPARA SI EXISTE EL NOMBRE DEL ARCHIVO EN LA TABLA DE ARCHIVOS,
 * 			POR LO QUE NO COMPRUEBA SI EL ARCHIVO ESTÁ BORRADO.
 * 			PARA COMPROBAR ESTO ULTIMO, DEBE USARSE OSADA_CHECH_EXIST (VEASE MÁS ABAJO)
 */
void* osada_get_file_called(char *path, t_disco_osada *disco);
t_list* osada_get_blocks_nums_of_this_file_since(int start_block);
int verificar_si_es_archivo_buscado(char *path, osada_file *file); //<-- Simple delegación de la función anterior
int comprobar_igualdad(char *path, osada_file *file);
int verificar_si_nombre_coincide(char *path, unsigned char* file_name);
int verificar_si_es_raiz(char *path);

int osada_b_check_parents(char *path, osada_file *file);

int es_par(int numero);
char* obtener_ruta_especifica(char *ruta_inicial, char *directorio_o_nombre_archivo, char *sub_directorio_o_nombre_archivo);
char* obtener_ruta_padre(char* path);
char* path_delete_last_reference(char *path_init);
char* path_first_reference(char *path);
/*---------------------------------------------VERIFICACION EXISTENCIA DE UN PATH-----------------------------------------*/
/*
 * @NAME: int osada_check_exist(char *path);
 * @DESC: Dado un PATH verifica su TOTAL existencia. Devuelve 1 caso positivo, 0 caso contrario.
 *
 * *NOTA: Un ejemplo de path puede ser: "/home/utnso/workspace/pepito.txt"
 * 			La función corrobora que existan cada uno de los directorios y que haya una correlatividad entre ellos,
 * 			además de revisar que NINGUNO de ellos esté borrado.
 * 			En el ejemplo mecionado, corrobora que: la carpeta "home" exista, que "utnso" exista y que su padre sea "home", etc.
 */
int osada_check_exist(char *path);
int verificar_existencia(char *file_or_directory, uint16_t dad_block); //<-- La usa la función anterior
int revisar_resultado(int result);//<-- La usa la función anterior
int existe_posta(void* result);

/*
 * @NAME: int calcular_posicion_en_tabla_de_archivos(int num_block, int position);
 * @DESC: Dado un número de bloque RELATIVO, y una posición (0 o 1), devolverá la posición exacta que tendrá en la TABLA DE ARCHIVOS
 *
 *  WARNING: RECUERDE QUE LA TABLA DE ARCHIVOS ES UN ARRAY, POR LO QUE LA POSICION DEVUELTA REPRESENTA LA POSICION EN EL ARRAY Y NO
 *  		EL NUM DE BYTE ABSOLUTO.
 *
 *  		EJEMPLO: calcular_posicion_en_tabla_de_archivos(1, 1); NOS DEVOLVERÁ LA POSICIÓN 3 DEL ARRAY.
 *  		NOTESE QUE LOS ARRAYS COMIENZAN EN 0. EL PRIMER BLOQUE (O) CONTIENE DOS POSICIONES (O,1),
 *  		EL SEGUNDO BLOQUE (1) CONTIENE DOS POSICIONES MAS (2,3), ....
 */
int calcular_posicion_en_tabla_de_archivos(int num_block, int position);
int calcular_bloque_en_tabla_de_archivos_segun_parent_directory(int parent);


void osada_b_rename_full(t_to_be_rename *to_be_rename);
void osada_b_rename(t_file_osada *file, char* new_nombre);
void renombrar_path_hijos(t_info_file *info_file_a_renombrar, char* new_path);

int osada_b_check_name(char* name);
char* obtener_nuevo_path(char* old_path, char* new_name);

int string_equals_osada_max_lenght(char* name, unsigned char* name_2);
void limpiar_bloque_de_datos(int n);

int verify_correct_file(osada_file *file);
t_list* osada_get_blocks_nums_of_this_file_Asco(osada_file *file, t_disco_osada *disco);
t_list* osada_get_blocks_nums_of_this_file_since_full(int start_block);
#endif /* FILE_MANIPULING_H_ */
