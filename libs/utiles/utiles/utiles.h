#ifndef UTILES_H_
#define UTILES_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>
#define REG_SIZE 4

/* Listado de los diferentes IDs de los mensajes a enviar en el sistema. */
typedef enum {
	CONSOLE_CODE,			/* Envio de codigo BESO de Consola a Kernel. */
	OK_MEMORY,				/* Memoria reservada en el MSP. */
	NOT_ENOUGH_MEMORY,		/* No hay memoria suficiente en el MSP para crear un segmento. */
	RESERVE_CODE,			/* Pedido de Kernel a reservar un segmento de codigo de un Programa a MSP. */
	RESERVE_STACK,			/* Pedido de Kernel a reservar un segmento de stack de un Programa a MSP. */
	WRITE_CODE,				/* Pedido de Kernel a escribir en memoria el codigo de un Programa a MSP. */
	CPU_TCB,			/* TCB enviado por el CPU luego de finalizada una ráfaga */
	OC_REQUEST,			/* Pedido del CPU a la MSP del siguiente código de operación */
	NEXT_OC,			/* Código de operación enviado por la MSP al CPU que lo solicitó */
	NEXT_TCB,			/* TCB enviado por el Kernel a una CPU disponible */
	ARG_REQUEST,			/* Pedido del CPU a la MSP del siguiente argumento */
	NEXT_ARG			/* Argumento enviado por la MSP al CPU que lo solicitó */
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

#ifndef T_HILO_
#define T_HILO_
typedef enum { NEW, READY, EXEC, BLOCK, EXIT } t_cola;

typedef struct {
	uint32_t pid;
	uint32_t tid;
	bool kernel_mode;
	uint32_t segmento_codigo;
	uint32_t segmento_codigo_size;
	uint32_t puntero_instruccion;
	uint32_t base_stack;
	uint32_t cursor_stack;
	int32_t registros[5];
	t_cola cola;
} __attribute__ ((__packed__)) t_hilo;
#endif

/* Funciones socket. */
int server_socket(uint16_t port);
int client_socket(char* ip, uint16_t port);
int accept_connection(int sockfd);
t_msg *new_message(t_msg_id id, char *message,uint32_t size);		/*Recibe un ID de tipo de mensaje y un puntero al stream a enviar. NO reserva memoria para el stream, usa el mismo puntero recibido*/
t_msg *recibir_mensaje(int sockfd);
void enviar_mensaje(int sockfd, t_msg *msg);
void destroy_message(t_msg *mgs);

/* Serializacion. */
char *serializar_tcb(t_hilo *tcb,uint16_t quantum);	/*Recibe un puntero al tcb y el quantum, retorna el stream listo a enviar*/	
void deserializar_tcb(t_hilo *tcb, char *stream);	/*Recibe un puntero al tcb y el stream del mensaje recibido. Guarda en el tcb la info recibida*/

/* Otros. */
int max(int a, int b);
long seedgen(void);
int randomize(int limit);
int msleep(useconds_t usecs);

/* Private functions. */
int _server_socket(uint16_t port, char *e_socket, char *e_setsockopt, char *e_bind, char *e_listen);
int _client_socket(char* ip, uint16_t port, char *e_socket, char *e_connect);
int _accept_connection(int sockfd, char *e_accept);
t_msg *_recibir_mensaje(int sockfd, char *e_recv);
void _enviar_mensaje(int sockfd, t_msg *msg, char *e_send);

#endif /* UTILES_H_ */
