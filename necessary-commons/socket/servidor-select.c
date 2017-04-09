/*
 * servidor-select.c
 *
 *  Created on: 14/11/2016
 *      Author: utnso
 */

#include "servidor-select.h"
			//*METODOS PRINCIPALES*//
t_socket_server* server_stream_create(int puerto, int backlog)
{
	t_socket_server *new_server = malloc(sizeof(t_socket_server));

	//Configuro direccion de servidor//
	address_config_in *direccionServidor =configurar_address_in_select(puerto);

	//Creo el socket para el server//
	int server_socket = socket(AF_INET,SOCK_STREAM,0);

	//Activo reutilización de direcciones//
	activar_reutilizacion_de_direcciones(1,server_socket);

	//Asocio socket al puerto por donde escuchará//
	server_stream_asociate_a_puerto(server_socket, direccionServidor);

	//Creo estructura readset para multiples clientes//
	fd_set *readset = readset_create();

	//Modelo al nuevo servidor//
	new_server->socket_asociado = server_socket;
	new_server->maximo_descriptor = new_server->socket_asociado;
	new_server->readset = readset;
	server_agrega_cliente_a_readset(new_server,new_server->socket_asociado);
	new_server->numero_clientes_conectados = 0;
	new_server->backlog = backlog;
	new_server->sockets_clientes_conectados = arrayInt_create(backlog);
	return new_server;

}

void server_stream_escucha(t_socket_server *server)
{
	printf("Estoy escuchando\n");
	listen(server->socket_asociado,server->backlog);
}

void server_stream_destruite(t_socket_server *server)
{
	free(server->readset);
	free(server->sockets_clientes_conectados);
	free(server->sockets_clientes_conectados);
	close(server->socket_asociado);
	free(server);
}

void server_stream_nuevo_cliente (t_socket_server *server,void (*funcion_de_atencion)(int parametro) )
{
	if(!server_permitis_nuevo_cliente(server->numero_clientes_conectados, server->backlog))
	{
		perror("Máxima cantidad de conectados. No se puede establecer conexión\n");
	}
	else
	{
		server_modela_nuevo_cliente(server);

	}
}

void server_stream_atende_clientes(t_socket_server *server,fd_set *descriptorTemporal,int longitudPaquete, void (*funcion_de_atencion)(int parametro))
{
	int socketCliente;
	//char* buffer = malloc(longitudPaquete);
	for(socketCliente=0; socketCliente<= (server->maximo_descriptor + 1) ; socketCliente++)
	{
		if(FD_ISSET(socketCliente, descriptorTemporal))
		{
			funcion_de_atencion(socketCliente);
			//server_atende_cliente(server,socketCliente,longitudPaquete, buffer);
		}

	}
	//free(buffer);
}



			//*METODOS SECUNDARIOS*//
int server_stream_asociate_a_puerto(int server, address_config_in *address)
{
	int resultBind;
	resultBind = bind(server,(void*) address,sizeof(*address));

	//Verificamos que no haya errores al asociar puerto//
	if(resultBind !=0)
	{	perror("Falló el bind");
		return 1;
	}
	return resultBind;
}

address_config_in* configurar_address_in_select(int puerto)
{
	address_config_in *direccionServidor = malloc(sizeof(address_config_in));
	direccionServidor->sin_family = AF_INET;
	direccionServidor->sin_addr.s_addr=INADDR_ANY;
	direccionServidor->sin_port=htons(puerto);
	memset(&(direccionServidor->sin_zero), '\0', 8);
	return direccionServidor;

}

fd_set* readset_create()
{
		fd_set *descriptoresDeLectura = malloc(sizeof(fd_set));
		FD_ZERO(descriptoresDeLectura); // <-- se limpian los bits de lectura
		return descriptoresDeLectura;

}

void readset_borrate(fd_set *readset)
{
	free(readset);
}

int server_permitis_nuevo_cliente(int nClientesConectados, int backlog)
{
	if(nClientesConectados == backlog)
	{
		return NO; //<-- no se permiten más clientes
	}
	return YES; //<-- se puede aceptar al cliente
}

void server_modela_nuevo_cliente(t_socket_server *server)
{
	/* Acepta la conexión con el cliente, guardándola en el array */
	int cliente_new = server_acepta_conexion_cliente_select(server->socket_asociado);

	if(cliente_new !=0)
	{
		server_aumenta_numero_de_conectados(server, cliente_new);
		server_agrega_cliente_a_readset(server, cliente_new);
	}
}

void server_aumenta_numero_de_conectados(t_socket_server *server, int cliente)
{
	server->numero_clientes_conectados = server->numero_clientes_conectados + 1;
	agregar_elemento_a_array(server->sockets_clientes_conectados,server->numero_clientes_conectados, cliente);
	server_cambia_tu_maximo_descriptor(server);
}

void server_cambia_tu_maximo_descriptor(t_socket_server *server)
{
	server->maximo_descriptor = arrayInt_maximo_entre(server->sockets_clientes_conectados,server->backlog, server->socket_asociado);
}

void server_disminui_numero_de_conectados(t_socket_server *server, int cliente)
{
	eliminar_elemento_del_array(server->sockets_clientes_conectados, server->numero_clientes_conectados, cliente);
	server->sockets_clientes_conectados = server->sockets_clientes_conectados - 1;
	server_cambia_tu_maximo_descriptor(server);
	server_quita_cliente_del_readset(server, cliente);
}

void server_agrega_cliente_a_readset(t_socket_server *server, int cliente)
{
	FD_SET(cliente, server->readset);
}

void server_quita_cliente_del_readset(t_socket_server *server, int cliente)
{
	FD_CLR(cliente, server->readset);
}

int server_acepta_conexion_cliente_select(int socketServer)
{
	address_config_in direccionCliente;
	unsigned int tamanioDireccion = sizeof(address_config_in);
	int cliente = accept(socketServer,(void*) &direccionCliente, &tamanioDireccion);
	if(cliente <0)
	{
		perror("Error al intentar aceptar cliente");
		close(cliente);
		return 0;
	}
	return cliente;
}

void server_atende_cliente(t_socket_server *server, int cliente, int longitudPaquete, char *buffer)
{
	int result;
	do
		{
			result = recv(cliente, buffer, longitudPaquete, 0);

		} while (result == -1);

	if(result >0)
		{
			printf("%s\n", buffer);
		}
	if(result == 0)
		{
			server_cerra_cliente_select(server, cliente);
		}
}

void server_cerra_cliente_select(t_socket_server *server, int cliente)
{
	printf("Se desconectó el cliente %d\n", cliente);
	close(cliente);
	server_disminui_numero_de_conectados(server,cliente);
}

void server_actualiza_maximo_descriptor(t_socket_server *server)
{
	server->maximo_descriptor=arrayInt_maximo_entre(server->sockets_clientes_conectados, server->backlog, server->socket_asociado);
}

void ejecutar_server_select(int puerto, int backlog, int longpaquete, void (*funcion_de_atencion)(int parametro))
{
		t_socket_server *server = server_stream_create(puerto, backlog);

		server_stream_escucha(server);

		fd_set *readset_temporal;
		readset_temporal=readset_create();

		//Ciclo infinito <<por ahora>> para escuchar peticiones de los clientes//
		while(1)
		{
			memcpy(readset_temporal, server->readset, sizeof(fd_set));

			int result;
			result=select((server->maximo_descriptor) + 1,readset_temporal, NULL, NULL, NULL);

			if(result >0)
			{
				if(FD_ISSET(server->socket_asociado, readset_temporal))
				{
					//server_stream_nuevo_cliente(server);
					FD_CLR(server->socket_asociado, readset_temporal);
					//funcion_de_atencion(result);
				}

				//server_stream_atende_clientes(server, readset_temporal, longpaquete,funcion_de_atencion);
				//funcion_de_atencion(result);
			}
		}
}


/*--------------------------------------------------AUXILIARES ARRAYS ------------------------------------------------------------*/
void agregar_elemento_a_array(int *array, int posicion, int elemento)
{
	array[posicion -1] = elemento;
}

void eliminar_elemento_del_array(int *array, int cantidadDeElementos, int elemento)
{
	int i;
	for(i=0; i<cantidadDeElementos; i++)
	{
		if(array[i]==elemento)
		{
			array[i]=0;
		}
	}
}

int mayor_del_array(int *array, int tamanio)
{
	int i;
	int mayor = 0;
	for (i=0; i<(tamanio -1); i++)
		{
		    if (array[i]> mayor) mayor=array[i];
		}
	return mayor;
}

int arrayInt_maximo_entre(int *array, int tamanioArray, int numero)
{
	int mayorDelArray = mayor_del_array(array,tamanioArray);
	if(mayorDelArray>numero)
		return mayorDelArray;
	else	return numero;
}

void inicializarArray(int *array, int tamanio)
{
	int posicion;
	for(posicion=0; posicion<tamanio; posicion++)
	{
		array[posicion]=0;
	}
}

int* arrayInt_create(int tamanio)
{
	int *array = malloc(sizeof(int) * tamanio);
	inicializarArray(array, tamanio);
	return array;
}

