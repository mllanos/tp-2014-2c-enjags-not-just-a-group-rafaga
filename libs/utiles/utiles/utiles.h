#ifndef UTILES_H_
#define UTILES_H_

#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

/* Creacion de sockets. */
int serverSocket(uint16_t port, struct sockaddr_in *name);
int clientSocket(char* ip, uint16_t port);

/* Envio y recibo de mensajes por sockets. */
void enviarMensaje(int sockfd, char* info);
t_mensaje* recibirMensaje(int sockfd);

/* Serializador. */
t_mensaje* deserializarMensaje(char* buffer, int nbytes);
t_stream* serializador(t_mensaje *mensaje);
t_mensaje* deserializador(t_stream *stream);

/* Otros. */
int obtenerMayor(int mayor, int numero);
long seedgen();
char intToChar(int i);
int randomizarNumero(int limite);

#endif /* UTILES_H_ */
