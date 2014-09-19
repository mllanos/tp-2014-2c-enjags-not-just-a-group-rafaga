#ifndef UTILES_H_
#define UTILES_H_

#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <commons/string.h>

/* Listado de los diferentes IDs de los mensajes a enviar en el sistema. */
typedef enum {
	CONSOLE_CODE,			/* Envio de codigo BESO de Consola a Loader */
	CONSOLE_OUT,			/* Fin de conexion de Consola a Loader */
	NEW_PROCESS,			/* Aviso del Kernel para crear un segmento para un nuevo proceso. */
	OK_MEMORY,				/* Memoria reservada en el MSP. */
	NOT_ENOUGH_MEMORY,		/* No hay memoria suficiente en el MSP. */
	RESERVE_CODE,			/* Pedido de Kernel a reservar codigo a MSP. */
	RESERVE_STACK			/* Pedido de Kernel a reservar stack a MSP. */
} t_msg_id;

/* Definicion de estructuras. */
typedef struct {
	t_msg_id id;
	uint16_t length;
}__attribute__ ((__packed__)) t_header;

typedef struct {
	t_header header;
	char *stream;
}__attribute__ ((__packed__)) t_msg;


/* Creacion de sockets. */
int server_socket(uint16_t port, struct sockaddr_in *name);
int client_socket(char* ip, uint16_t port);

/* Envio y recibo de mensajes por sockets. */
t_msg *new_message(t_msg_id id, char *message);
t_msg *recibir_mensaje(int sockfd);
void enviar_mensaje(int sockfd, t_msg *msg);
void destroy_message(t_msg *mgs);

/* Otros. */
int max(int a, int b);
long seedgen(void);
int randomize(int limit);
int msleep(useconds_t usecs);

#endif /* UTILES_H_ */
