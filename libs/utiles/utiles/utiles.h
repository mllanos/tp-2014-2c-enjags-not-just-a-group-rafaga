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
#include <panel/panel.h>

#define REG_SIZE 4


/****************** IDS DE MENSAJES. ******************/

typedef enum {
	NO_NEW_ID,					/* Valor centinela para evitar la modificacion de id en modify_message(). */

	INIT_CONSOLE,				/* Pedido de creacion de hilo principal de Consola a Kernel. */
	KILL_CONSOLE,				/* Respuesta de finalizacion por error de Kernel a Consola. */

	CPU_CONNECT,				/* Pedido de conexion de CPU a Kernel. */

	CPU_TCB,					/* Pedido de TCB de CPU a Kernel. */
	NEXT_THREAD,				/* Envio de TCB de Kernel a CPU. */

	RETURN_TCB,					/* Retorno de TCB de CPU a Kernel. */


	/****************** SERVICIOS EXPUESTOS A CPU: INTERRUPCION. ******************/ 
	CPU_INTERRUPT,				/* Pedido de interrupcion de un hilo de proceso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: ENTRADA ESTANDAR. ******************/
	NUMERIC_INPUT,				/* Pedido de input numerico de CPU. */
	STRING_INPUT,				/* Pedido de input de string de CPU. */
	REPLY_INPUT,				/* Respuesta de input de Consola. */

	/****************** SERVICIOS EXPUESTOS A CPU: SALIDA ESTANDAR. ******************/
	STRING_OUTPUT,				/* Pedido de salida estandar de CPU. */

	/****************** SERVICIOS EXPUESTOS A CPU: CREAR HILO. ******************/
	CPU_THREAD,					/* Pedido de nuevo hilo de proceso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: JOIN. ******************/
	CPU_JOIN,					/* Pedido de union a hilo de proceso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: BLOQUEAR. ******************/
	CPU_BLOCK,					/* Pedido de bloqueo de hilo de proceso por recurso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: DESPERTAR. ******************/
	CPU_WAKE,					/* Pedido de desbloqueo hilo de proceso bloqueado por recurso de CPU a Kernel. */


	/****************** INTERFAZ MSP: RESERVAR SEGMENTO. ******************/
	RESERVE_SEGMENT,			/* Pedido de reserva de segmento a MSP. */
	OK_RESERVE,					/* Respuesta de memoria reservada de MSP. */
	ENOMEM_RESERVE,				/* Respuesta de memoria insuficiente de MSP. */

	/****************** INTERFAZ MSP: DESTRUIR SEGMENTO. ******************/
	DESTROY_SEGMENT,			/* Pedido de destruccion de segmento a MSP. */

	/****************** INTERFAZ MSP: SOLICITAR MEMORIA. ******************/
	MEM_REQUEST,				/* Pedido de datos a la MSP. */
	OK_REQUEST,					/* Respuesta de lectura correcta de MSP. */
	SEGFAULT_REQUEST,			/* Respuesta de error de segmento en lectura de MSP. */

	/****************** INTERFAZ MSP: ESCRIBIR MEMORIA. ******************/
	WRITE_MEMORY,				/* Pedido de escritura en memoria a MSP. */
	OK_WRITE,					/* Respuesta de escritura correcta de MSP. */
	SEGFAULT_WRITE,				/* Respuesta de error de segmento en escritura de MSP. */


	/****************** DEPRECADOS. ******************/
	OC_REQUEST,					/* Pedido del CPU a la MSP del siguiente código de operación. */ 
	NEXT_OC,					/* Código de operación enviado por la MSP al CPU que lo solicitó. */
	ARG_REQUEST,				/* Pedido del CPU a la MSP del siguiente argumento. */
	NEXT_ARG,					/* Argumento enviado por la MSP al CPU que lo solicitó. */
	WRITE_MEM					/* Pedido de escritura en memoria a la MSP. */
} t_msg_id;


/****************** ESTRUCTURAS DE DATOS. ******************/

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

/*
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
*/

/****************** FUNCIONES SOCKET. ******************/

/*
 * Crea, vincula y escucha un socket desde un puerto determinado.
 */
int server_socket(uint16_t port);

/*
 * Crea y conecta a una ip:puerto determinado.
 */
int client_socket(char* ip, uint16_t port);

/*
 * Acepta la conexion de un socket.
 */
int accept_connection(int sock_fd);

/*
 * Recibe un t_msg a partir de un socket determinado.
 */
t_msg *recibir_mensaje(int sock_fd);

/*
 * Envia los contenidos de un t_msg a un socket determinado.
 */
void enviar_mensaje(int sock_fd, t_msg *msg);


/****************** FUNCIONES T_MSG. ******************/

/*
 * Crea un t_msg a partir de un string y count argumentos.
 */
t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...);

/*
 * Agrega nuevos argumentos a un mensaje (estilo FIFO).
 */
t_msg *modify_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...);

/*
 * Elimina todos los argumentos existentes de un mensaje y agrega nuevos.
 */
t_msg *remake_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...);

/*
 * Crea un t_msg a partir de los contenidos de un archivo beso y count argumentos.
 */
t_msg *beso_message(t_msg_id id, char *beso_path, uint16_t count, ...);

/*
 * Crea un t_msg a partir de los contenidos de un tcb y count argumentos.
 */
t_msg *tcb_message(t_msg_id id, t_hilo *tcb, uint16_t count, ...);

/*
 * Crea un mensaje (viejo).
 */
t_msg *crear_mensaje(t_msg_id id, char *message, uint32_t size);

/*
 * Libera los contenidos de un t_msg.
 */
void destroy_message(t_msg *mgs);


/****************** SERIALIZACION TCB. ******************/ 

/*
 * Recibe un puntero al tcb y el quantum, retorna el stream listo a enviar.
 */
char *serializar_tcb(t_hilo *tcb,uint16_t quantum); // TODO QUITAR QUANTUM DE ACA

/*
 * Recibe un puntero al tcb y el stream del mensaje recibido. Guarda en el tcb la info recibida.
 */
void deserializar_tcb(t_hilo *tcb, char *stream);


/****************** FUNCIONES AUXILIARES. ******************/

/*
 * Compara dos numeros y retorna el maximo.
 */
int max(int a, int b);

/*
 * Genera una nueva secuencia de enteros pseudo-random a retornar por rand().
 */
void seedgen(void);

/*
 * RNG. Retorna valores entre 0 y limit.
 */
int randomize(int limit);

/*
 * Sleep en microsegundos.
 */
int msleep(useconds_t usecs);

/*
 * Lee un archivo y retorna sus contenidos.
 */
char *read_file(char *path);

/*
 * Alternativa sin undefined behavior a fflush(STDIN) para usar con scanf().
 */
void clean_stdin_buffer(void);

/*
 * Muestra los contenidos y argumentos de un t_msg.
 */
void putmsg(t_msg *msg);

/*
 * Funcion auxiliar para putmsg().
 */
char *id_string(t_msg_id id);

/*
 * Recupera los contenidos de un tcb cargado a mensaje.
 */
t_hilo *retrieve_tcb(t_msg *msg);

#endif /* UTILES_H_ */
