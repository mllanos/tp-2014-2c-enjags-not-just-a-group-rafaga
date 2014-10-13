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

/* Funciones Macro */

/*
 * Compara dos numeros y retorna el mínimo.
 */
#define min(n,m) (n < m ? n : m)

/*
 * Compara dos numeros y retorna el máximo.
 */
#define max(n,m) (n > m ? n : m)

/*
 * Sleep en microsegundos.
 */
#define msleep(usecs) usleep(usecs*1000)

/*
 * RNG. Retorna valores entre 0 y limit.
 */
#define randomize(limit) (rand() % (limit + 1))

/*
 * Divide dos enteros redondeando hacia arriba.
 */
#define divRoundUp(n,m) (n % m == 0 ? n / m : n / m + 1)

/* FIN Funciones Macro */

/****************** IDS DE MENSAJES. ******************/

typedef enum {
	NO_NEW_ID,				/* Valor centinela para evitar la modificación de id en modify_message(). */

	INIT_CONSOLE,				/* Pedido de creación de hilo principal de Consola a Kernel. */
	KILL_CONSOLE,				/* Respuesta de finalización por error de Kernel a Consola. */

	CPU_CONNECT,				/* Pedido de conexión de CPU a Kernel. */

	CPU_TCB,				/* Pedido de TCB de CPU a Kernel. */
	NEXT_THREAD,				/* Envío de TCB de Kernel a CPU. */

	RETURN_TCB,					/* Retorno de TCB de CPU a Kernel. */


	/****************** SERVICIOS EXPUESTOS A CPU: INTERRUPCIÓN. ******************/ 
	CPU_INTERRUPT,				/* Pedido de interrupción de un hilo de proceso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: ENTRADA ESTÁNDAR. ******************/
	NUMERIC_INPUT,				/* Pedido de input numérico de CPU. */
	STRING_INPUT,				/* Pedido de input de string de CPU. */
	REPLY_INPUT,				/* Respuesta de input de Consola. */

	/****************** SERVICIOS EXPUESTOS A CPU: SALIDA ESTÁNDAR. ******************/
	STRING_OUTPUT,				/* Pedido de salida estándar de CPU. */

	/****************** SERVICIOS EXPUESTOS A CPU: CREAR HILO. ******************/
	CPU_THREAD,				/* Pedido de nuevo hilo de proceso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: JOIN. ******************/
	CPU_JOIN,				/* Pedido de unión a hilo de proceso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: BLOQUEAR. ******************/
	CPU_BLOCK,				/* Pedido de bloqueo de hilo de proceso por recurso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: DESPERTAR. ******************/
	CPU_WAKE,				/* Pedido de desbloqueo hilo de proceso bloqueado por recurso de CPU a Kernel. */


	/****************** INTERFAZ MSP: RESERVAR SEGMENTO. ******************/
	CREATE_SEGMENT,				/* Pedido de creación de segmento a MSP. */
	OK_CREATE,				/* Respuesta de segmento creado de MSP. */
	FULL_MEMORY,				/* Respuesta de memoria llena. */
	INVALID_SEG_SIZE,			/* Respuesta de tamaño de segmento inválido. */
	MAX_SEG_NUM_REACHED,			/* Respuesta de máxima cantidad de segmentos del proceso alcanzada. */

	/****************** INTERFAZ MSP: DESTRUIR SEGMENTO. ******************/
	DESTROY_SEGMENT,			/* Pedido de destrucción de segmento a MSP. */

	/****************** INTERFAZ MSP: SOLICITAR MEMORIA. ******************/
	REQUEST_MEMORY,				/* Pedido de datos a la MSP. */
	OK_REQUEST,				/* Respuesta de lectura correcta de MSP. */

	/****************** INTERFAZ MSP: ESCRIBIR MEMORIA. ******************/
	WRITE_MEMORY,				/* Pedido de escritura en memoria a MSP. */
	OK_WRITE,				/* Respuesta de escritura correcta de MSP. */

	/****************** INTERFAZ MSP: SOLICITAR/ESCRIBIR MEMORIA. ******************/
	INVALID_DIR,				/* Respuesta de dirección inválida. */
	SEGMENTATION_FAULT,			/* Respuesta de error de segmento en lectura/escritura de memoria. */

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
char *serializar_tcb(t_hilo *tcb,uint16_t quantum);

/*
 * Recibe un puntero al tcb y el stream del mensaje recibido. Guarda en el tcb la info recibida.
 */
void deserializar_tcb(t_hilo *tcb, char *stream);


/****************** FUNCIONES AUXILIARES. ******************/

/*
 * Genera una nueva secuencia de enteros pseudo-random a retornar por rand().
 */
void seedgen(void);

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
