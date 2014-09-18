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

//definiendo que codigo corresponde a que tipo de mensaje (en msg.encabezado.codMsg)
#define U_PEDIDO_BYTES              0      //mensaje a umv pidiendo el contenido de una posicion
#define U_ALMACENAR_BYTES           1      //mensaje a umv pidiendo que se graben datos en una posicion
#define KERNEL_HANDSHAKE            2      //mensaje de kernel a umv de handshake indicandole que es de tipo kernel
#define CPU_HANDSHAKE               3      //mensaje de cpu a umv de handshake indicandole que es de tipo cpu
#define K_HANDSHAKE                 4      //handshake de cpu a kernel
#define C_PEDIDO_OK                 5      //respuesta de UMV de un pedido general ok
#define K_PEDIDO_VAR_GL             6      //pedido de cpu a kernel por el valor de una variable global
#define K_ASIGNAR_VAR_GL            7      //pedido de cpu a kernel para que asigne un valor a una variable global
#define K_IMPRIMIR_TXT              8      //pedido de cpu a kernel para que imprima un texto (un *char que le envia)
#define K_EXPULSADO_ES             10      //notificacion de cpu a kernel que el proceso quiere haces e/s durante una cantidad de tiempo
#define K_WAIT                     11      //notificacion de cpu a kernel que el proceso hace wait a un semaforo
#define K_SIGNAL                   12      //notificacion de cpu a kernel que el proceso hace signal a un semaforo
#define U_PROCESO_ACTIVO           13      //notificacion de cpu a UMV de cambio de proceso activo en la cpu
#define U_CREAR_SEGMENTO           14      //mensaje a umv para que cree un segmento
#define U_DESTRUIR_SEGMENTO        15      //mensaje a umv para que destruya un segmento
#define K_CREACION_SEGMENTO_OK     16      //mensaje a kernel contestando por la creacion de un segmento
#define K_CREACION_SEGMENTO_FAIL   17      //mensaje a kernel contestando por la creacion de un segmento
#define CONEXION_OK                18      //mensaje de umv a kernel o cpu contestando el handshake
#define K_ENVIO_SCRIPT             19      //mensaje de programa a kernel para envio del codigo
#define P_ENVIAR_SCRIPT            20      //mnesaje de kernel a programa pidiendo el codigo script
#define DATOS_ALMACENADOS_OK       21      //mensaje de umv a kernel o cpu de datos almacenados correctamente
#define DESBORDE_MEMORIA           22      //mensaje de umv a kernel o cpu de intento de acceso a memoria que no le pertenece
#define C_ENVIO_PCB                23      //mensaje de kernel a cpu para enviarle pcb+quantum+retardo
#define K_IMPRIMIR_VAR             24      //mensaje de cpu a kernel para que imprima un valor
#define VALOR_COMPARTIDA_OK        25      //mensaje de kernel a cpu con el valor de la variable compartida solicitada
#define C_WAIT_OK                  26      //mensaje de kernel a cpu indicando que no se bloquea el proceso en ese semaforo
#define P_SERVICIO_DENEGADO        27      //mensaje de kernel a programa denegandole servicio por problemas de memoria
#define K_EXPULSADO_FIN_PROG       28      //mensaje de cpu a kernel informando que el programa llego a su fin
#define K_EXPULSADO_FIN_QUANTUM    29      //mensaje de cpu a kernel para que actualice el pcb antes de que sea desalojado del cpu
#define K_EXPULSADO_SEG_FAULT      30      //mensaje de cpu a kernel informando que la lectura/escritura del programa produce segmentation fault
#define K_EXPULSADO_DESCONEXION    31      //mensaje de cpu a kernel informando que se va a desconectar
#define K_EXPULSADO_WAIT           32      //mensaje de cpu a kernel informando que se expulsa al proceso por wait en un semaforo
#define U_EXPULSADO_DESCONEXION    33      //mensaje de cpu a umv informando que se desconectara
#define P_DESCONEXION              34      //mensaje de kernel a programa informando que lo desconecta
#define PEDIDO_NOK                 35
#define P_IMPRIMIR_VAR             36      //mensaje de kernel a programa para que imprima el valor de una variable
#define P_IMPRIMIR_TXT             37      //mensaje de kernel a programa para que imprima un texto
#define C_SIGNAL_OK                38      //mensaje de kernel a cpu informando que la operacion signal fue ok
#define C_ERROR                    39      //mensaje de kernel a cpu informando varCompartida, semaforo, o dispositivo de E/S inexistentes

/*
typedef struct mensaje {
	char id;
	char* info;
}__attribute__((__packed__)) t_mensaje;

typedef struct stream {
	int length;
	char* data;
}__attribute__((__packed__)) t_stream;
*/

typedef struct {
	uint8_t id;
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
t_msg *new_message(uint8_t id, char *message);
t_msg *recibir_mensaje(int sockfd);
void enviar_mensaje(int sockfd, t_msg *msg);
void destroy_message(t_msg *mgs);
//void enviarMensaje(int sockfd, char* info);
//t_mensaje* recibirMensaje(int sockfd);

/* Serializador. */
//t_mensaje* deserializarMensaje(char* buffer, int nbytes);
//t_stream* serializador(t_mensaje *mensaje);
//t_mensaje* deserializador(t_stream *stream);


/* Otros. */
int max(int a, int b);
long seedgen(void);
int randomize(int limit);
int msleep(useconds_t usecs);

#endif /* UTILES_H_ */
