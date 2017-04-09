/*
 * serializacion_pokedex_cliente_servidor.c
 *
 *  Created on: 6/10/2016
 *      Author: utnso
 */
#include "serializacion_pokedex_cliente_servidor.h"

/*-----------------------------------------PETICIONES--------------------------------------------*/
char* build_msg(int header, char *path_original, char *path_new_or_text, int size, int offset)
{
	switch(header)
	{
		case(LISTAR):
			{
				char *msg = armar_header(LISTAR);
				char *bytes_path = armar_numero_de_bytes(string_length(path_original));
				string_append(&msg,bytes_path );
				string_append(&msg, path_original);
				free(bytes_path);
				return msg;
			};break;
		case(GET_ATRIBUTES):
			{
				char *msg = armar_header(GET_ATRIBUTES);
				char *num_bytes_path = armar_numero_de_bytes(string_length(path_original));
				string_append(&msg, num_bytes_path);
				string_append(&msg, path_original);
				free(num_bytes_path);
				return msg;
			};break;
		case(CREATE_FILE):
			{
				char *msg = armar_header(CREATE_FILE);
				char *num_bytes_path = armar_numero_de_bytes(string_length(path_original));
				string_append(&msg, num_bytes_path);
				string_append(&msg, path_original);
				free(num_bytes_path);
				return msg;
			};break;
		case(CREATE_DIRECTORY):
			{
				char *msg = armar_header(CREATE_DIRECTORY);
				char *num_bytes_path =armar_numero_de_bytes(string_length(path_original));
				string_append(&msg,num_bytes_path);
				string_append(&msg, path_original);
				free(num_bytes_path);
				return msg;
			};break;
		case(DELETE_FILE):
			{
				char *msg = armar_header(DELETE_FILE);
				char *num_bytes_path = armar_numero_de_bytes(string_length(path_original));
				string_append(&msg,num_bytes_path);
				string_append(&msg, path_original);
				free(num_bytes_path);
				return msg;
			};break;
		case(DELETE_DIRECTTORY):
			{
				char *msg = armar_header(DELETE_DIRECTTORY);
				char *num_bytes_path =  armar_numero_de_bytes(string_length(path_original));
				string_append(&msg,num_bytes_path);
				string_append(&msg, path_original);
				free(num_bytes_path);
				return msg;
			};break;
		case(READ_FILE):
			{
				char *msg = armar_header(READ_FILE);
				char *resto = armar_lectura_o_escritura(READ_FILE,path_original,NULL,size,offset);
				string_append(&msg,resto);
				free(resto);
				return msg;
			};break;
		case(WRITE_FILE):
			{
				char *header = armar_header(WRITE_FILE);
				char *mensaje = malloc(string_length(path_original) + 30 +2);
				memcpy(mensaje,header,2);

				free(header);

				char *msg_model = armar_numero_de_bytes(string_length(path_original));
				string_append(&msg_model,path_original);

				char *size_a_escribir = armar_numero_de_bytes(size); //OJO ACA
				string_append(&msg_model, size_a_escribir);
				free(size_a_escribir);

				char *a_partir_de = armar_numero_de_bytes(offset);
				string_append(&msg_model,a_partir_de);
				free(a_partir_de);

				int offset_interno = 30 + string_length(path_original);
				memcpy(mensaje+2,msg_model,offset_interno);
				free(msg_model);
				return mensaje;
			};break;
		case(RENAME_FILE):
			{
				char *msg = armar_header(RENAME_FILE);
				char *num_bytes_first_path = armar_numero_de_bytes(string_length(path_original));
				string_append(&msg, num_bytes_first_path);
				string_append(&msg, path_original);
				char *num_bytes_second_path = armar_numero_de_bytes(string_length(path_new_or_text));
				string_append(&msg,num_bytes_second_path);
				string_append(&msg, path_new_or_text);
				free(num_bytes_first_path);
				free(num_bytes_second_path);
				return msg;
			};break;
		case(OPEN_FILE):
			{
				char *msg = armar_header(OPEN_FILE);
				char *num_bytes_path = armar_numero_de_bytes(string_length(path_original));
				string_append(&msg, num_bytes_path);
				string_append(&msg, path_original);
				free(num_bytes_path);
				return msg;
			};break;
		case(11):
			{
				char *msg = armar_header(11);
				char *num_bytes_path = armar_numero_de_bytes(string_length(path_original));
				string_append(&msg, num_bytes_path);
				string_append(&msg, path_original);

				char *size_string = armar_numero_de_bytes(size);
				string_append(&msg,size_string);

				free(num_bytes_path);
				free(size_string);
				return msg;
			}
	}
}

char* armar_numero_de_bytes(int size_payload)
{
	char *size_String = string_itoa(size_payload);
	int length = string_length(size_String);

	char *result = string_repeat(' ',MAX_BYTES_TO_SEND-length);
	string_append(&result,size_String);
	free(size_String);
	return result;
}

char* armar_header(int header)
{
	if(header < 10)
	{
		char *heaader = string_repeat(' ',1);
		char *itoa = string_itoa(header);
		string_append(&heaader,itoa);
		free(itoa);
		return heaader;
	}
	else
	{
		return string_itoa(header);
	}
}

char* armar_lectura_o_escritura(int tipo,char *path, char *text, int size, int offset)
{
	switch(tipo)
	{
		case(READ_FILE):
		{
			char *msg = armar_numero_de_bytes(string_length(path));
			string_append(&msg,path);
			char *size_a_leer = armar_numero_de_bytes(size);
			string_append(&msg, size_a_leer);
			char *a_partir_de = armar_numero_de_bytes(offset);
			string_append(&msg,a_partir_de);
			free(size_a_leer);
			free(a_partir_de);
			return msg;
		};break;
		case(WRITE_FILE):
		{
			char *msg_model = armar_numero_de_bytes(string_length(path));
			string_append(&msg_model,path);


			int tamanio_texto = string_length(text);
			char *size_a_escribir = armar_numero_de_bytes(tamanio_texto); //OJO ACA
			string_append(&msg_model, size_a_escribir);
			free(size_a_escribir);

			char *a_partir_de = armar_numero_de_bytes(offset);
			string_append(&msg_model,a_partir_de);
			free(a_partir_de);

			int offset_interno = 30 + string_length(path);

			char *mensaje = malloc(string_length(text) + 30 + size);
			memcpy(mensaje,msg_model,offset_interno);
			free(msg_model);

			memcpy(mensaje +offset_interno,text,size);
			return mensaje;
		};break;
	}
}

/*-----------------------------------------RESPUESTAS-------------------------------------------------*/
void* recibir_mensaje_escritura(int socket, int payloadsize)
{
	void *payload = malloc(payloadsize);
	int bytes_recibidos = 0;
	bytes_recibidos = recv(socket,payload,payloadsize,0);

	int last_offset  = 0;
	if(bytes_recibidos<payloadsize)
	{
		while(bytes_recibidos<payloadsize)
		{
			int tamanio = payloadsize-bytes_recibidos;
			int offset_anterior = bytes_recibidos;
			void *pay_aux = malloc(tamanio);
			bytes_recibidos = recv(socket,pay_aux,tamanio,0) + bytes_recibidos;

			int nuevo_tamanio = bytes_recibidos - offset_anterior;
			memcpy(payload + offset_anterior,pay_aux,nuevo_tamanio);
			free(pay_aux);
			last_offset = bytes_recibidos;
		}
	}

	char* feof= malloc(20);
	int bytes_recibidos_feof = recv(socket, feof,19,0);
	if(string_equals_ignore_case(feof,"FIN_ESCRITURA_OSADA"))
	{
		return payload;
	}

}
