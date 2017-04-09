/*
 * sockets-utils.c
 *
 *  Created on: 25/9/2016
 *      Author: utnso
 */

#include "sockets-utils.h"

address_config_in configurar_address_in(int puerto, char *ip)
{
	address_config_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr=inet_addr(ip);
	direccionServidor.sin_port=htons(puerto);
	memset(&(direccionServidor.sin_zero), '\0', 8);
	return direccionServidor;
}

void activar_reutilizacion_de_direcciones(int valorVerdaderoso, int socket)
{
	setsockopt(socket,SOL_SOCKET,SO_REUSEADDR, &valorVerdaderoso, sizeof(valorVerdaderoso));
}

int sendall(int s, char *buf, int *len)
 {
        int total = 0;        // cuántos bytes hemos enviado
        int bytesleft = *len; // cuántos se han quedado pendientes
        int n;
        while(total < *len) {
            n = send(s, buf+total, bytesleft, 0);
            if (n == -1) { break; }
            total += n;
            bytesleft -= n;
        }
        *len = total; // devuelve aquí la cantidad enviada en realidad
        return n==-1?-1:0; // devuelve -1 si hay fallo, 0 en otro caso
 }

void enviar_mensaje_cantidad_especifica(int socket, void *buffer, int size)
{
	int cantidad_total_enviada = 0;
	cantidad_total_enviada = send(socket,buffer,size,0);
	if(cantidad_total_enviada < size)
	{
		while(cantidad_total_enviada < size)
		{
			void *data = malloc(size-cantidad_total_enviada);
			memcpy(data,buffer+cantidad_total_enviada,size-cantidad_total_enviada);
			cantidad_total_enviada = seguir_enviado(socket, data,size-cantidad_total_enviada) + cantidad_total_enviada;
			free(data);
		}
	}
}

int seguir_enviado(int socket, void *buffer, int size)
{
	int cantidad_enviada;
	cantidad_enviada=send(socket,buffer,size,0);
	return cantidad_enviada;
}

void enviar_mensaje(int socket, char *msg)
{
	send(socket, msg, strlen(msg),0);
}


void* reciveall(int socket_num, int len)
{
	int offset = 0;
	int len_restante = len;
	int n=0;
	void* buffer = malloc(len);
	memset(buffer, 0, len+1); // Limpiamos el buffer

	while(n<len)
	{
		n  = recv(socket_num,buffer+offset,len_restante, 0);
		offset = n;
		len_restante = len-n;
	}
	return buffer;
}


char* recibir_mensaje(int socket,int payloadSize)
{
	char *payload = malloc(payloadSize+1);
	int bytes_recibidos;
	bytes_recibidos=recv(socket, payload, payloadSize,0);
	if(bytes_recibidos == 0)
	{
		free(payload);
		char *desconectado = string_new();
		string_append(&desconectado, "DESCONECTADO");
		return desconectado;
	}
	else
	{
		payload[payloadSize]= '\0';
		return payload;
	}

}

void* recibir_mensaje_tipo_indistinto(int socket,int payloadsize)
{
	void *payload = malloc(payloadsize);
	int bytes_recibidos = 0;
	bytes_recibidos = recv(socket,payload,payloadsize,0);
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
		}
	}
	return payload;
}


char* recibir_mensaje_tipo_indistinto_string(int socket,int payloadsize)
{
	char *payload = malloc(payloadsize);
	limpiar_buff_aux(payload,payloadsize);
	int bytes_recibidos = 0;
	bytes_recibidos = recv(socket,payload,payloadsize,0);
	if(bytes_recibidos<payloadsize)
	{
		while(bytes_recibidos<payloadsize)
		{
			int tamanio = payloadsize-bytes_recibidos;
			int offset_anterior = bytes_recibidos;
			void *pay_aux = malloc(tamanio);
			limpiar_buff_aux(pay_aux,payloadsize);

			bytes_recibidos = recv(socket,pay_aux,tamanio,0) + bytes_recibidos;

			int nuevo_tamanio = bytes_recibidos - offset_anterior;
			memcpy(payload + offset_anterior,pay_aux,nuevo_tamanio);
			free(pay_aux);
		}
	}
	return payload;
}

void limpiar_buff_aux(char* buff,int tamanio)
{
	int i;
	for(i=0;i<tamanio;i++)
		{
			buff[i]='\0';
		}
}



void* recibir_mensaje_tipo_indistinto_2(int socket,int payloadsize)
{
		void *payload = malloc(payloadsize);
		int bytes_recibidos = 0;
		int offset = 0;

		while(bytes_recibidos != payloadsize)
		{
			bytes_recibidos = recv(socket,payload+offset,payloadsize,0) + bytes_recibidos;
			offset = payloadsize-bytes_recibidos;
		}
		return payload;
}
