#ifndef UTILES_H_
#define UTILES_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>


#define REG_SIZE 4 /* Tamaño de los registros de la CPU */

/* Listado de los diferentes IDs de los mensajes a enviar en el sistema. */
typedef enum {
	INIT_CONSOLE,				/* Pedido de creacion de hilo principal de Consola a Kernel. */
	KILL_CONSOLE,				/* Respuesta de finalizacion por error de Kernel a Consola. */

	RESERVE_SEGMENT,			/* Pedido de reserva de segmento a MSP. */
	OK_RESERVE,					/* Respuesta de memoria reservada de MSP. */
	ENOMEM_RESERVE,				/* Respuesta de memoria insuficiente de MSP. */

	WRITE_MEMORY,				/* Pedido de escritura en memoria a MSP. */
	OK_WRITE,					/* Respuesta de escritura correcta de MSP. */
	SEGFAULT_WRITE,				/* Respuesta de error de segmento en escritura de MSP. */

	OC_REQUEST,					/* Pedido del CPU a la MSP del siguiente código de operación */
	NEXT_OC,					/* Código de operación enviado por la MSP al CPU que lo solicitó */
	NEXT_THREAD,				/* TCB enviado por el Kernel a una CPU disponible */
	ARG_REQUEST,				/* Pedido del CPU a la MSP del siguiente argumento */
	NEXT_ARG,					/* Argumento enviado por la MSP al CPU que lo solicitó */
	MEM_REQUEST,				/* Pedido de datos a la MSP */
	WRITE_MEM,					/* Pedido de escritura en memoria a la MSP */

	CPU_TCB,					/* TCB devuelto por la CPU */
	CPU_CONNECT,				/* Pedido de conexion de CPU a Kernel. */
	CPU_PROCESS,				/* Pedido de conexion a proceso de CPU a Kernel. */
	CPU_DISCONNECT,				/* Pedido de desconexion de CPU a Kernel. */
	CPU_INTERRUPT,				/* Pedido de interrupcion de un hilo de proceso de CPU a Kernel. */
	CPU_INPUT,					/* Pedido de entrada estandar de CPU a Kernel. */
	CPU_OUTPUT,					/* Pedido de salida estandar de CPU a Kernel. */
	CPU_THREAD,					/* Pedido de nuevo hilo de proceso de CPU a Kernel. */
	CPU_JOIN,					/* Pedido de union a hilo de proceso de CPU a Kernel. */
	CPU_BLOCK,					/* Pedido de bloqueo de hilo de proceso por recurso de CPU a Kernel. */
	CPU_WAKE					/* Pedido de removido de bloqueo a los hilos de procesos bloquedos por recurso de CPU a Kernel. */
} t_msg_id;

/* Definicion de estructuras. */
typedef struct {
	t_msg_id id;
	uint16_t length;
	uint16_t argc;
}__attribute__ ((__packed__)) t_header;

typedef struct {
	t_header header;
	char *stream;
	uint32_t *argv;
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
t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...);
t_msg *modify_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...);
t_msg *beso_message(t_msg_id id, char *beso_path, uint16_t count, ...);
t_msg *crear_mensaje(t_msg_id id, char *message, uint32_t size); /*Recibe un ID de tipo de mensaje, un puntero al stream a enviar, y su tamaño. NO reserva memoria para el stream, usa el mismo puntero recibido*/
t_msg *recibir_mensaje(int sockfd);
t_msg *_recibir_mensaje(int sockfd);
void enviar_mensaje(int sockfd, t_msg *msg);
void _enviar_mensaje(int sockfd, t_msg *msg);
void destroy_message(t_msg *mgs);


/* Serializacion. */
char *serializar_tcb(t_hilo *tcb,uint16_t quantum);	/*Recibe un puntero al tcb y el quantum, retorna el stream listo a enviar*/	
void deserializar_tcb(t_hilo *tcb, char *stream);	/*Recibe un puntero al tcb y el stream del mensaje recibido. Guarda en el tcb la info recibida*/


/* Funciones auxiliares. */
int max(int a, int b);
long seedgen(void);
int randomize(int limit);
int msleep(useconds_t usecs);
char *read_file(char *path);
void putmsg(t_msg *msg);

#endif /* UTILES_H_ */
