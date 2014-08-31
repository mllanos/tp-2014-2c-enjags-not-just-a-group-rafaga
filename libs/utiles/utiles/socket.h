/*
 * socket.h
 *
 *  Created on: 04/06/2013
 *      Author: utnso
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

	void socket_crear(int* sockfd);
	void socket_setopt(int sockfd, int* yes);
	void socket_info_mi_direc(struct sockaddr_in* mi_direc, int puerto);
	void socket_bind(int sockfd, struct sockaddr_in* mi_direc);
	void socket_escuchar(int sockfd);
	int socket_aceptar(int sockfd, struct sockaddr_in* p_dest_direc, socklen_t * p_addrlen );
	void socket_info_dest_direc(struct sockaddr_in* dest_direc, int puerto, char* ip);
	void socket_conectar(int sockfd, struct sockaddr_in* p_dest_direc);
	int socket_recibir(int unSocket, char* buffer);
	int socket_enviar(int unSocket, char* data, int length);
	void socket_cerrar(int socket);

#endif /* SOCKET_H_ */
