/*
 * utiles.h
 *
 *  Created on: Oct 19, 2013
 *      Author: mario
 */

#ifndef UTILES_H_
#define UTILES_H_

	#include <time.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/types.h>
	#include <unistd.h>

	extern int msleep(__useconds_t __useconds);

	typedef struct mensaje {
		char id;
		char* info;
	}__attribute__((__packed__)) t_mensaje;

	typedef struct stream {
		int length;
		char* data;
	}__attribute__((__packed__)) t_stream;

	int crearSocket(char* ip, int puerto);

	void enviarMensaje(int sockfd, char* info);
	t_mensaje* recibirMensaje(int sockfd);
	t_mensaje* descerializarMensaje(char* buffer, int nbytes);

	t_stream* serializador(t_mensaje *mensaje);
	t_mensaje* descerializador(t_stream *stream);

	int obtenerMayor(int mayor, int numero);
	long seedgen();
	char intToChar(int i);
	int randomizarNumero(int limite);

#endif /* UTILES_H_ */
