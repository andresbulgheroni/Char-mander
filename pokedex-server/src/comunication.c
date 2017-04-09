/*
 * comunication.c
 *
 *  Created on: 6/10/2016
 *      Author: utnso
 */
#include "comunication.h"

int se_ejecuta = 1;
extern t_log *logger;

void loggear_resultado(int resultado)
{
	switch(resultado)
	{
	case(EXITO): printf("EXITO!\n");break;
	case(NO_EXISTE): printf("NO EXISTE\n");break;
	case(EXISTE): printf("EL NOMBRE YA EXISTE\n");break;
	case(NO_HAY_ESPACIO): printf("NO HAY ESPACIO\n");break;
	case(ARGUMENTO_INVALIDO):printf("ARGUMENTO INVALIDO\n");break;
	}
}

/*--------------------------------------CONEXION--------------------------------------------------------------*/
void ejecutar_servidor()
{
	pokedex_server_conectate();
	printf("Listo para escuchar peticiones!\n");
	while(se_ejecuta >0)
	{
		int cliente = server_acepta_conexion_cliente(servidor_pokedex);

			//COMO ACCEPT ES BLOQUEANTE --> SI ESTÁ EN ESTE PUNTO ES QUE YA HAY UN CLIENTE ONLINE
			servidor_osada_crea_nuevo_cliente(&cliente);

	}
}

void pokedex_server_conectate()
{
	char *ip_string = getenv("IP_POKEMON");
	char *puerto_string = getenv("PUERTO_POKEMON");
	if(ip_string==NULL || puerto_string==NULL)
	{
		printf("Necesito IP_POKEMON y PUERTO_POKEMON para ejecutar\n");
		exit(1);
	}
	else
	{
		int puerto = atoi(puerto_string);
		servidor_pokedex = server_create(puerto, ip_string, 1500);
		server_escucha(servidor_pokedex);
		//printf("%s",ip_string);
	}


	/*char *ip = string_new();
	string_append(&ip,"127.0.0.1");
	servidor_pokedex = server_create(5001, ip, 1500);
	free(ip);
	server_escucha(servidor_pokedex);*/
}

void servidor_osada_crea_nuevo_cliente(int* cliente)
{
		pthread_attr_t attr;
		pthread_t thread;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		pthread_create(&thread,NULL,server_pokedex_atende_cliente,(void*)cliente);

		pthread_attr_destroy(&attr);
}

void* server_pokedex_atende_cliente(void* socket_cliente)
{
	int* conversion= (int*) socket_cliente;
	int cliente = *conversion;
	int cliente_esta_conectado = 1;
	printf("NUEVO CLIENTE SOCKET: %d\n", cliente);

	while(cliente_esta_conectado)
	{
		char *peticion = server_escucha_peticion(cliente);
		if(string_equals_ignore_case(peticion, "DESCONECTADO"))
		{
			printf("Cliente número %d desconectado\n", cliente);
			cliente_esta_conectado = 0;
			free(peticion);
		}
		else
		{
			tratar_peticion_de(cliente, peticion);
			free(peticion);
		}
	}
	server_cerra_cliente(cliente);
	pthread_exit(NULL);
}

char* server_escucha_peticion(int cliente)
{

	char *peticion = recibir_mensaje(cliente,HEADER);
	return peticion;
}

void tratar_peticion_de(int cliente,char *peticion)
{
	int peticion_header = atoi(peticion);

	switch (peticion_header)
	{
		case(LISTAR):
		{
			char *path = recibir_mensaje_especifico(cliente, LISTAR);
			printf("CLIENTE %d PIDE LISTAR: %s\n",cliente,path);
			void* resultado = osada_a_get_list_dir(path);

			if((int) resultado == NO_EXISTE)
			{
				loggear_resultado(resultado);
				enviar_mensaje(cliente,"FFFF");
			}
			else
			{
				char* bytes_to_send = string_new();
				char *listado_string = armar_listado((t_list*) resultado,&bytes_to_send);
				//enviar_mensaje(cliente, bytes_to_send);
				free(bytes_to_send);
				enviar_mensaje_cantidad_especifica(cliente, listado_string,string_length(listado_string));
				free(listado_string);

			}
			free(path);
		};break;
		case(GET_ATRIBUTES):
		{
			printf("CLIENTE %d PIDE ATRIBUTOS DE: ", cliente);
			char *path = recibir_mensaje_especifico(cliente, GET_ATRIBUTES);
			printf("%s\n",path);
			void* respuesta = osada_a_get_attributes(path);

			if((int) respuesta == NO_EXISTE)
			{
				enviar_mensaje(cliente,"F");
				free(path);
			}
			else
			{
				t_attributes_file *file = (t_attributes_file*) respuesta;
				char *mensj = armar_attributes(file);
				enviar_mensaje(cliente, mensj);
				free(file);
				free(path);
				free(mensj);
			}
		};break;
		case(CREATE_FILE):
		{
			char *path = recibir_mensaje_especifico(cliente, CREATE_FILE);
			printf("CLIENTE %d PIDE CREAR FILE: %s\n",cliente,path);
			int resultado_operacion = osada_a_create_file(path);
			loggear_resultado(resultado_operacion);
			responder_solo_resultado(cliente,resultado_operacion);

			free(path);
		};break;
		case(CREATE_DIRECTORY):
		{
			char *path = recibir_mensaje_especifico(cliente, CREATE_DIRECTORY);
			printf("CLIENTE %d PIDE CREAR DIR: %s\n",cliente,path);
			int resultado_operacion = osada_a_create_dir(path);
			loggear_resultado(resultado_operacion);
			responder_solo_resultado(cliente,resultado_operacion);
			free(path);
		};break;
		case(DELETE_FILE):
		{
			char *path = recibir_mensaje_especifico(cliente, DELETE_FILE);
			printf("CLIENTE %d PIDE DELETE: %s\n",cliente,path);
			int resultado_operacion = osada_a_delete_file(path);
			loggear_resultado(resultado_operacion);
			responder_solo_resultado(cliente,resultado_operacion);
			free(path);
		};break;
		case(DELETE_DIRECTTORY):
		{
			char *path = recibir_mensaje_especifico(cliente, DELETE_DIRECTTORY);
			printf("CLIENTE %d PIDE DELETE: %s\n",cliente,path);
			int resultado_operacion = osada_a_delete_dir(path);
			loggear_resultado(resultado_operacion);
			responder_solo_resultado(cliente,resultado_operacion);
			free(path);
		};break;
		case(READ_FILE):
		{
			t_to_be_read *file_to_read = recibir_mensaje_especifico(cliente, READ_FILE);
			printf("CLIENTE %d PIDE LEER: %d BYTES OFFSET: %d DE: %s\n", cliente,file_to_read->size,file_to_read->offset,file_to_read->path);
			void *result = osada_a_read_file(file_to_read);

			if((int)result == NO_EXISTE || (int) result == ARGUMENTO_INVALIDO)
			{
				enviar_mensaje(cliente,"FFFFFFFFFF");
				loggear_resultado(result);
			}
			else
			{
				read_content *resultado = (read_content*) result;
				printf("LECTURA COMPLETADA: %d BYTES DE: %s\n",resultado->tamanio ,file_to_read->path);
				char *tam = string_itoa(resultado->tamanio);
				int tamanio_del_archivo = string_length(tam);
				char *mensaje = string_repeat(' ', 10 -tamanio_del_archivo);
				string_append(&mensaje,tam);
				free(tam);
				enviar_mensaje(cliente,mensaje);

				int tamanio_final = resultado->tamanio;
				sendall(cliente,resultado->contenido, &tamanio_final);
				free(mensaje);
				free(resultado->contenido);
				free(resultado);
			}
			free(file_to_read->path);
			free(file_to_read);
		};break;
		case(WRITE_FILE):
		{
			t_to_be_write *file_to_write = recibir_mensaje_especifico(cliente,WRITE_FILE);
			printf("CLIENTE %d PIDE ESCRIBIR: %s ,SIZE: %d OFFSET: %d\n", cliente,file_to_write->path,file_to_write->size,file_to_write->offset);
			int resultado = osada_a_write_file(file_to_write);
			loggear_resultado(resultado);
			responder_solo_resultado(cliente,resultado);
			free(file_to_write->path);
			free(file_to_write->text);
			free(file_to_write);
		};break;
		case(RENAME_FILE):
		{
			t_to_be_rename *file_to_rename= recibir_mensaje_especifico(cliente, RENAME_FILE);
			printf("CLIENTE %d PIDE RENAME: %s\n",cliente,file_to_rename->old_path);
			int resultado_operacion = osada_a_rename(file_to_rename);
			loggear_resultado(resultado_operacion);
			responder_solo_resultado(cliente,resultado_operacion);
			free(file_to_rename->new_path);
			free(file_to_rename->old_path);
			free(file_to_rename);
		};break;
		case(OPEN_FILE):
		{
			char *path = recibir_mensaje_especifico(cliente, OPEN_FILE);
			printf("CLIENTE %d PIDE ABRIR: %s\n",cliente,path);
			int resultado_operacion = osada_a_open_file(path);
			responder_solo_resultado(cliente,resultado_operacion);
			free(path);
		};break;
		case(11):
		{
			t_to_be_truncate* to_truncate = recibir_mensaje_especifico(cliente,11);
			printf("CLIENTE %d PIDE TRUNCAR: %s CON EL SIZE %d bytes \n",cliente,to_truncate->path,to_truncate->new_size);
			int resultado_operacion = osada_a_truncate_file(to_truncate->path,to_truncate->new_size);
			responder_solo_resultado(cliente,resultado_operacion);
			free(to_truncate->path);
			free(to_truncate);
		};break;
		default:printf ("%s\n",peticion);
	}
}

/*--------------------------------------PETICIONES DEL CLIENTE-----------------------------------------------------*/
void* recibir_mensaje_especifico(int socket, int header)
{
	switch(header)
		{
			case(LISTAR):
				{
					return escuchar_mensaje_operaciones_basicas(socket);
				};break;
			case(GET_ATRIBUTES):
				{
					return escuchar_mensaje_operaciones_basicas(socket);
				};break;
			case(CREATE_FILE):
				{
					return escuchar_mensaje_operaciones_basicas(socket);
				};break;
			case(CREATE_DIRECTORY):
				{
					return escuchar_mensaje_operaciones_basicas(socket);
				};break;
			case(DELETE_FILE):
				{
					return escuchar_mensaje_operaciones_basicas(socket);
				};break;
			case(DELETE_DIRECTTORY):
				{
					return escuchar_mensaje_operaciones_basicas(socket);
				};break;
			case(READ_FILE):
				{
					return escuchar_mensaje_read(socket);
				};break;
			case(WRITE_FILE):
				{
					return escuchar_mensaje_write(socket);
				};break;
			case(RENAME_FILE):
				{
					return escuchar_mensaje_rename(socket);
				};break;
			case(OPEN_FILE):
				{
					return escuchar_mensaje_operaciones_basicas(socket);
				};break;
			case(11):
				{
					return escuchar_mensaje_truncate(socket);
				};break;
		}
}

char* escuchar_mensaje_operaciones_basicas(int socket)
{
	char *bytes_of_path = recibir_mensaje(socket,BYTES_TO_RCV);
	int bytes_path = atoi(bytes_of_path);
	free(bytes_of_path);
	char *path = recibir_mensaje(socket, bytes_path);
	return path;
}

t_to_be_read* escuchar_mensaje_read(int socket)
{
	t_to_be_read *to_read = malloc(sizeof(t_to_be_read));

	char *bytes_of_path = recibir_mensaje(socket,BYTES_TO_RCV);
	int bytes_path = atoi(bytes_of_path);
	free(bytes_of_path);

	char *path = recibir_mensaje(socket, bytes_path);
	to_read->path = path;

	char *size_to_be_read = recibir_mensaje(socket, BYTES_TO_RCV);
	int size_to_read = atoi(size_to_be_read);
	free(size_to_be_read);
	to_read->size = size_to_read;

	char *offset_string = recibir_mensaje(socket, BYTES_TO_RCV);
	int offset = atoi(offset_string);
	free(offset_string);
	to_read->offset = offset;

	return to_read;
}

t_to_be_write* escuchar_mensaje_write(int socket)
{
	t_to_be_write *to_write = malloc(sizeof(t_to_be_write));

	char *bytes_of_path = recibir_mensaje(socket,BYTES_TO_RCV);
	int bytes_path = atoi(bytes_of_path);

	free(bytes_of_path);

	char *path = recibir_mensaje(socket, bytes_path);
	to_write->path = path;

	char *size_to_be_write = recibir_mensaje(socket, BYTES_TO_RCV);
	int size_to_write= atoi(size_to_be_write);
	free(size_to_be_write);
	to_write->size = size_to_write;

	char *offset_string = recibir_mensaje(socket, BYTES_TO_RCV);
	int offset = atoi(offset_string);
	free(offset_string);
	to_write->offset = offset;


	void *text = recibir_mensaje_tipo_indistinto(socket,size_to_write);
	to_write->text = text;

	return to_write;

}

t_to_be_rename* escuchar_mensaje_rename(int socket)
{
	t_to_be_rename *to_rename = malloc(sizeof(t_to_be_rename));

	char *bytes_of_path = recibir_mensaje(socket,BYTES_TO_RCV);
	int bytes_path = atoi(bytes_of_path);
	free(bytes_of_path);
	char *old_path = recibir_mensaje(socket, bytes_path);
	to_rename->old_path =old_path;

	char *bytes_of_path_2= recibir_mensaje(socket,BYTES_TO_RCV);
	int bytes_path_2 = atoi(bytes_of_path_2);
	free(bytes_of_path_2);
	char *new_path = recibir_mensaje(socket, bytes_path_2);
	to_rename->new_path =new_path;

	return to_rename;
}

t_to_be_truncate* escuchar_mensaje_truncate(int socket)
{
	t_to_be_truncate* to_truncate = malloc(sizeof(t_to_be_truncate));

	char *bytes_of_path = recibir_mensaje(socket,BYTES_TO_RCV);
	int bytes_path = atoi(bytes_of_path);
	free(bytes_of_path);
	char *path = recibir_mensaje(socket, bytes_path);

	to_truncate->path = path;

	char *size_to_be_truncate = recibir_mensaje(socket, BYTES_TO_RCV);
	int size_to_truncate = atoi(size_to_be_truncate);
	free(size_to_be_truncate);

	to_truncate->new_size =size_to_truncate;

	return to_truncate;
}


/*--------------------------------------RESPUESTAS AL CLIENTE----------------------------------------------------*/
void responder_solo_resultado(int cliente, int resultado)
{
	char *resultado_a_enviar = string_itoa(resultado);
	enviar_mensaje(cliente, resultado_a_enviar);
	free(resultado_a_enviar);
}

char* armar_attributes(t_attributes_file *attributes)
{
	char *size = string_itoa(attributes->size);
	char *tipo = string_itoa(attributes->tipo);
	int tamanio = string_length(size);

	char *msg = string_repeat(' ', 10-tamanio);
	string_append(&msg,size);
	string_append(&msg,tipo);

	free(size);
	free(tipo);
	return msg;
}

char* armar_listado(t_list *listado,char** num_bytes_so_send)
{
	char* listado_string = string_new();
	if(list_is_empty(listado))
	{
		string_append(&listado_string, "0000");
		list_destroy(listado);
		return listado_string;
	}
	else
	{
		int size = list_size(listado);
		//modelar_cantidad_elementos_listado(listado_string,size);
		int total_bytes=0;
		char *list=modelar_elementos_en_listado(listado,size, &total_bytes);

		int total_total = total_bytes + (size*2) +4;
		char* total_bytes_a_enviar_string = string_itoa(total_total);
		int tamanio_del_total = string_length(total_bytes_a_enviar_string);

		char* total_a_enviar_final = string_repeat(' ',15-tamanio_del_total);
		string_append(&total_a_enviar_final,total_bytes_a_enviar_string);
		free(total_bytes_a_enviar_string);
		string_append(num_bytes_so_send,total_a_enviar_final);
		free(total_a_enviar_final);

		//string_append(&listado_string,total_a_enviar_final);
		modelar_cantidad_elementos_listado(listado_string,size);
		string_append(&listado_string,list);
		free(list);
		list_destroy_and_destroy_elements(listado,file_listado_eliminate);
		return listado_string;
	}

}

void modelar_cantidad_elementos_listado(char* buffer, int size)
{
	char *size_string = string_itoa(size);
	if(size < 10)
	{
		char* repeat = string_repeat(' ',3);
		string_append(&buffer,repeat);
		string_append(&buffer,size_string);
		free(repeat);
		free(size_string);
	}
	else
	{
		if(size<100)
		{
			char* repeat = string_repeat(' ',2);
			string_append(&buffer,repeat);
			string_append(&buffer,size_string);
			free(repeat);
			free(size_string);
		}
		else
		{
			if(size<1000)
			{
				char* repeat = string_repeat(' ',1);
				string_append(&buffer,repeat);
				string_append(&buffer,size_string);
				free(repeat);
				free(size_string);
			}
			else
			{
				string_append(&buffer,size_string);
				free(size_string);
			}
		}
	}
}

char* modelar_elementos_en_listado(t_list *listado, int size, int* total_bytes_a_enviar)
{
	char *resultado = string_new();
	int i=0;
	while(i<size)
	{
		t_file_listado *file = list_get(listado,i);
		//char* last_element = array_last_element(file->path);
		int tamanio_nombre = string_length(file->path);

		*total_bytes_a_enviar=*total_bytes_a_enviar + (tamanio_nombre -1);

		char *size_name=modelar_tamanio_nombre(tamanio_nombre-1);
		string_append(&resultado,size_name);
		free(size_name);
		//string_append(&resultado,last_element);
		char* elemento = string_substring(file->path,1,tamanio_nombre);
		string_append(&resultado,elemento);
		free(elemento);
		//free(last_element);
		i++;
	}
	return resultado;
}

char* modelar_tamanio_nombre(int size)
{
	if(size < 10)
	{
		char *repeat=string_repeat(' ',1);
		char *size_string = string_itoa(size);
		string_append(&repeat, size_string);
		free(size_string);
		return repeat;
	}
	else
	{
		char *size_string = string_itoa(size);
		return size_string;
	}
}

void agregar_barra_si_es_necesario(char *path)
{
	if(!string_starts_with(path,"/"))
	{
		char *aux = string_new();
		string_append(&aux,"/");
		string_append(&aux, path);
		path=realloc(path,string_length(path) + 2);
		path = aux;
		free(aux);
	}
}
