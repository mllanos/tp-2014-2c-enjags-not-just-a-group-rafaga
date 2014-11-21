#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <semaphore.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <utiles/utiles.h>
#include <panel/kernel_log.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include "loader.h"
#include "planificador.h"

#define PANEL_PATH "panel/"
#define LOG_PATH "log/kernel.log"
#define MAXEVENTS 64

/* Macros de respuesta MSP. */
#define MSP_RESERVE_FAILURE(s) (s == FULL_MEMORY || s == INVALID_SEG_SIZE || s == MAX_SEG_NUM_REACHED)
#define MSP_RESERVE_SUCCESS(s) (s == OK_CREATE)
#define MSP_WRITE_FAILURE(s) (s == INVALID_DIR || s == SEGMENTATION_FAULT)
#define MSP_WRITE_SUCCESS(s) (s == OK_WRITE)

/* Macros getters de config. */
#define get_puerto() config_get_int_value(config, "PUERTO")
#define get_ip_msp() config_get_string_value(config, "IP_MSP")
#define get_puerto_msp() config_get_int_value(config, "PUERTO_MSP")
#define get_quantum() config_get_int_value(config, "QUANTUM")
#define get_stack_size() config_get_int_value(config, "TAMANIO_STACK")
#define get_syscalls() config_get_string_value(config, "SYSCALLS")


typedef enum {
	CONSOLE_ID, 
	CPU_ID 
} t_unique_id;

/* Funciones Kernel. */

/*
 * Inicializa variables globales.
 */
void initialize(char *config_path);

/*
 * Libera los recursos de las variables globales.
 */
void finalize(void);

/*
 * Crea el TCB Kernel y lo encola en BLOCK.
 */
void boot_kernel(void);

/*
 * Recibe las conexiones y mensajes de CPU y Consola.
 */
void receive_messages_epoll(void);
void receive_messages_select(void);

/*
 * Interpreta mensajes y los asigna a las colas compartidas.
 */
void interpret_message(int sock_fd, t_msg *recibido);

/*
 * Se encarga de pedir memoria a MSP para un proceso inicial.
 */
t_hilo *reservar_memoria(t_hilo *tcb, t_msg *msg);

/*
 * Se encarga de sacar de las listas a los CPUs y Consolas salientes.
 * En caso de ser una CPU con el TCB Kernel finaliza el programa.
 */
int remove_from_lists(uint32_t sock_fd);


/* Funciones auxiliares. */
uint32_t get_unique_id(t_unique_id id);
int make_socket_non_blocking(int sfd);


/* Archivos de configuracion y log. */
t_config *config;
t_log *logger;


/* Listas. */
t_list *process_list;
t_list *console_list;
t_list *cpu_list;


/* Diccionarios. */
t_dictionary *join_dict;
t_dictionary *resource_dict;
t_dictionary *father_child_dict;


/* Colas. */
t_queue *loader_queue;
t_queue *planificador_queue;
t_queue *syscall_queue;
t_queue *request_queue;


/* Mutex. */
pthread_mutex_t loader_mutex;
pthread_mutex_t planificador_mutex;
pthread_mutex_t unique_id_mutex[3];
pthread_mutex_t console_list_mutex;
pthread_mutex_t cpu_list_mutex;
pthread_mutex_t process_list_mutex;


/* Semaphores. */
sem_t sem_loader;
sem_t sem_planificador;


/* Hilos. */
pthread_t loader_th;
pthread_t planificador_th;


/* File descriptors. */
int msp_fd;


/* TCB de hilo de Kernel. */
t_hilo *klt_tcb;

#endif
