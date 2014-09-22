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
#define REG_SIZE 4 /* Tamaño de los registros de la CPU */

/* Listado de los diferentes IDs de los mensajes a enviar en el sistema. */
typedef enum {
	CONSOLE_CODE,			/* Envio de codigo BESO de Consola a Kernel. */
	INIT_CONSOLE, /* Pedido de creacion de hilo principal de Consola a Kernel. */	
	KILL_CONSOLE, /* Respuesta de finalizacion por error de Kernel a Consola. */
	RESERVE_CODE, /* Pedido de reserva de segmento de codigo de Kernel a MSP. */
	RESERVE_STACK, /* Pedido de reserva de segmento de stack de Kernel a MSP. */
	OK_MEMORY, /* Respuesta de memoria reservada de MSP a Kernel. */
	NOT_ENOUGH_MEMORY, /* Respuesta de memoria insuficiente de MSP a Kernel. */
	WRITE_CODE, /* Pedido de escritura de codigo BESO de Kernel a MSP. */
	OK_CODE, /* Respuesta de escritura correcta de codigo BESO de MSP a Kernel. */
	OC_REQUEST,			/* Pedido del CPU a la MSP del siguiente código de operación */
	NEXT_OC,			/* Código de operación enviado por la MSP al CPU que lo solicitó */
	NEXT_THREAD,			/* TCB enviado por el Kernel a una CPU disponible */
	ARG_REQUEST,			/* Pedido del CPU a la MSP del siguiente argumento */
	NEXT_ARG,			/* Argumento enviado por la MSP al CPU que lo solicitó */
	CPU_CONNECT, /* Pedido de conexion de CPU a Kernel. */
	CPU_INTERRUPT, /* Pedido de interrupcion de un hilo de proceso de CPU a Kernel. */
	CPU_INPUT, /* Pedido de entrada estandar de CPU a Kernel. */
	CPU_OUTPUT, /* Pedido de salida estandar de CPU a Kernel. */
	CPU_THREAD, /* Pedido de nuevo hilo de proceso de CPU a Kernel. */
	CPU_JOIN, /* Pedido de union a hilo de proceso de CPU a Kernel. */
	CPU_BLOCK, /* Pedido de bloqueo de hilo de proceso por recurso de CPU a Kernel. */
	CPU_WAKE /* Pedido de removido de bloqueo a los hilos de procesos */
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
