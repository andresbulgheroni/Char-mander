/*
 * comunication.c
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */
#include "comunication.h"
/*--------------------------------------CONEXION CON EL SERVIDOR--------------------------------------------------*/
int conectar_a_mapa(t_mapa *mapa)
{
	mapa->server= cliente_create(atoi(mapa->puerto),mapa->ip);
	return mapa->server;
}

int mapa_me_dice(char *mapa_dice)
{
	if(string_equals_ignore_case(mapa_dice, "sr;"))
	{
		return MAPA_ME_DA_POKEMON;
	}
	if(string_equals_ignore_case(mapa_dice, "mr;"))
	{
		return MAPA_ME_DA_MEDALLA;
	}
	if(string_equals_ignore_case(mapa_dice, "bq;"))
	{
		return MAPA_ME_BLOQUEA;
	}
	if(string_equals_ignore_case(mapa_dice, "fb;"))
	{
			return MAPA_ME_DESBLOQUEA;
	}
	if(string_equals_ignore_case(mapa_dice, "ur;"))
	{
		return MAPA_ME_DA_COORDENADAS_POKENEST;
	}
	if(string_equals_ignore_case(mapa_dice, "mpk"))
	{
		return MAPA_ME_AVISA_DEADLOCK;
	}
	if(string_equals_ignore_case(mapa_dice, "gnr"))
	{
		return MAPA_ME_DICE_QUE_GANE;
	}
	if(string_equals_ignore_case(mapa_dice, "prd"))
	{
		return MAPA_ME_DICE_QUE_PERDI;
	}
	if(string_equals_ignore_case(mapa_dice,"tr;"))
	{
		return MAPA_ME_DA_TURNO;
	}
	if(string_equals_ignore_case(mapa_dice, "123"))
	{
		return MAPA_ME_PREGUNTA_SI_ESTOY;
	}
	if(string_equals_ignore_case(mapa_dice,"DESCONECTADO"))
	{
		return MAPA_DESCONECTADO;
	}
	else return MAPA_DESCONECTADO;
}

void enviar_mensaje_a_mapa(t_mapa *mapa, int header, char *payload)
{
	switch(header)
	{
		case(OTORGAR_SIMBOLO_ENTRENADOR): enviar_mensaje(mapa->server,payload);break;
		case(SOLICITAR_COORDENADAS_POKENEST): solicitar_ubicacion_pokenest(mapa, payload);break;
		case(SOLICITAR_CAPTURA_POKEMON):enviar_mensaje(mapa->server, "cp;");break;
		case(REPORTAR_MOVIMIENTO): solicitar_moverse(mapa, payload);break;
		case(REPORTAR_FIN_OBJETIVOS):enviar_mensaje(mapa->server, "fp;");break;
		case(ENTREGAR_MEJOR_POKEMON): enviar_mensaje(mapa->server,payload);break;
		case(SIGO_VIVO):enviar_mensaje(mapa->server,"123");break;
	}
}

char* escuchar_mensaje_mapa(t_mapa *mapa, int header)
{
	switch(header)
	{
		case(MAPA_ME_DA_TURNO): return( recibir_mensaje(mapa->server, 3)); break;
		case(MAPA_ME_AVISA_QUE_ME_VA_A_ENVIAR): return(recibir_mensaje(mapa->server,3)); break;
		case(MAPA_ME_DA_POKEMON): return(recibir_mensaje_especifico(mapa->server)); break;
		case(MAPA_ME_DA_COORDENADAS_POKENEST): return(recibir_mensaje_especifico(mapa->server)); break;
		case(MAPA_ME_DA_MEDALLA): return(recibir_mensaje_especifico(mapa->server)); break;
		case(MAPA_ME_BLOQUEA): return(recibir_mensaje(mapa->server, 3)); break;
		case(MAPA_ME_DESBLOQUEA): return(recibir_mensaje(mapa->server,3));break;
		default: return("NO SE PUEDE INTERPRETAR");
	}
}

void entrenador_dale_a_mapa_tu_simbolo()
{
	extern t_entrenador *entrenador;
	enviar_mensaje(entrenador->mapa_actual->server, entrenador->simbolo);
}

/*---------------------------------------MENSAJES QUE ENVIA EL ENTRENADOR AL MAPA----------------------------------------*/
void solicitar_ubicacion_pokenest(t_mapa *mapa,char *pokemonBuscado)
{
	char *mensaje = armar_mensaje("up", pokemonBuscado,MAX_BYTES_TOTAL_A_ENVIAR);
	enviar_mensaje(mapa->server, mensaje);
	free(mensaje);
}

void solicitar_moverse(t_mapa *mapa,char *coordenadaDestino)
{
	char *mensaje = armar_mensaje("mp", coordenadaDestino, MAX_BYTES_TOTAL_A_ENVIAR);
	enviar_mensaje(mapa->server, mensaje);
	free(mensaje);
}

char* armar_mejor_pokemon_string(t_pokemon *pokemon)
{
	char *msg = string_new();
		int size_name_pokemon = string_length(pokemon->species);

		char *size_name_string = string_itoa(size_name_pokemon);
		char *name_pokemon_Aux = string_repeat(' ',14-string_length(size_name_string));

		string_append(&name_pokemon_Aux,size_name_string);
		free(size_name_string);

		string_append(&name_pokemon_Aux, pokemon->species);

		string_append(&msg,name_pokemon_Aux);
		free(name_pokemon_Aux);

		char* level_String = string_itoa(pokemon->level);
		int size_level = string_length(level_String);

		char* level_aux = string_repeat(' ',6-size_level);
		string_append(&level_aux, level_String);

		string_append(&msg, level_aux);
		free(level_aux);
		free(level_String);

		//log_info(info_entrenador,msg);

		/* 14 bytes para saber size nombre del pokemon, N bytes del nombre del pokemon, 6 bytes para el level */

		return msg;


}
/*---------------------------------------SECUNDARIOS----------------------------------------*/
t_ubicacion* desarmar_coordenada(char *coordenada)
{
	char **por_separado = string_split(coordenada, ";");
	t_ubicacion *ubi = ubicacion_create(atoi(por_separado[0]),atoi(por_separado[1]));
	array_free_all(por_separado);
	return (ubi);
}

void copiar(char* origen, char* destino)
{
	char *origen_aux = string_new();
	string_append(&origen_aux, origen);
	char * origen_aux_2=string_path_replace_spaces(&origen_aux, " ", "\\ ");

	char *destino_aux = string_new();
	string_append(&destino_aux,destino);
	char* destino_aux_2=string_path_replace_spaces(&destino_aux, " ","\\ ");

	char* mensaje = string_new();
	string_append(&mensaje, "cp ");
	string_append(&mensaje, origen_aux_2);
	string_append(&mensaje, " ");
	string_append(&mensaje, destino_aux_2);

	system(mensaje);

	free(mensaje);
	free(destino_aux);
	free(origen_aux);
	free(origen_aux_2);
	free(destino_aux_2);
}

void eliminar(char* elemento)
{
	char* aux = string_new();
	aux=string_path_replace_spaces(&elemento, " ", "\\ ");

	char* mensaje = string_new();
	string_append(&mensaje, "rm ");
	string_append(&mensaje, aux);
	system(mensaje);
	free(mensaje);
	free(aux);
}

char* string_path_replace_spaces(char **path, char *este_caracter, char *por_este)
{

		char **por_separado = string_split(*path,"/");
		int i = 0;
		char *aux = string_new();
		int cantidad_elementos = 0;
		while(por_separado[i] != NULL)
		{
			cantidad_elementos++;
			i++;
		}

		i=0;
		while(por_separado[i] != NULL)
		{
			if( i == (cantidad_elementos -1))
			{
				if(string_ends_with(*path, "/"))
				{
					if(string_contains(por_separado[i],este_caracter))
					{
						char *aux_2 = string_new();
						string_append(&aux_2, por_separado[i]);
						char* auxiliar = string_replace(&aux_2,este_caracter,por_este);
						string_append(&aux, auxiliar);
						string_append(&aux,"/");
						free(aux_2);
						free(auxiliar);
					}
					else
					{
						string_append(&aux, por_separado[i]);
					}
				}
				else
				{
					if(string_contains(por_separado[i],este_caracter))
					{
						char *aux_2 = string_new();
						string_append(&aux_2, por_separado[i]);
						char* auxiliar = string_replace(&aux_2,este_caracter,por_este);
						string_append(&aux, auxiliar);
						string_append(&aux,"/");
						free(aux_2);
						free(auxiliar);
					}
					else
					{
						string_append(&aux, por_separado[i]);
					}
				}
			}
			else
			{
				if(i==0 && string_starts_with(*path, "/"))
				{
					string_append(&aux, "/");
					char *aux_2 = string_new();
					string_append(&aux_2, por_separado[i]);

					if(string_contains(por_separado[i],este_caracter))
					{
						char* auxiliar=string_replace(&aux_2,este_caracter,por_este);
						string_append(&aux, auxiliar);
						free(auxiliar);
					}
					else
					{
						string_append(&aux, aux_2);
					}
					string_append(&aux,"/");
					free(aux_2);
				}
				else
				{
					char *aux_2 = string_new();
					string_append(&aux_2, por_separado[i]);
					if(string_contains(por_separado[i],este_caracter))
					{
						char* auxiliar =string_replace(&aux_2,este_caracter,por_este);
						string_append(&aux,auxiliar);
						free(auxiliar);
					}
					else
					{
						string_append(&aux,aux_2);
					}
					string_append(&aux,"/");
					free(aux_2);
				}
			}
			i++;
		}

		free_string_array(por_separado);
		return aux;

}

void borrar_todos_los_archivos_del_directorio(char* ruta)
{
	char* comando = string_new();
	string_append(&comando, "rm -rf ");

	char* path_aux = string_path_replace_spaces(&ruta, " ", "\\ ");
	string_append(&comando, path_aux);
	if(!string_ends_with(comando,"/"))
	{
		string_append(&comando, "/*");
	}
	else
	{
		string_append(&comando, "*");
	}

	system(comando);

	//char* comando_2 = string_new();
	//string_append(&comando_2, "mkdir -p ");
	//string_append(&comando_2,path_aux);

	//system(comando_2);

	free(comando);
	//free(comando_2);
	free(path_aux);
}

char* array_last_element(char* path)
{
	char **file_for_file = string_split(path, "/");
	int size = array_size(file_for_file);
	char *nombre = string_new();
	string_append(&nombre,file_for_file[size - 1]);
	array_free_all(file_for_file);
 	return nombre;
}

int array_size(char **array)
{
	int cantidad_de_elementos = 0;
	int i = 0;
	while(array[i] !=NULL)
	{
		cantidad_de_elementos++;
		i++;
	}
	return cantidad_de_elementos;
}

void array_free_all(char **array)
{
	int i =0;
	while(array[i] != NULL)
	{
		free(array[i]);
		i++;
	}
	free(array);
}
