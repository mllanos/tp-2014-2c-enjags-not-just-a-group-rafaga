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
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <utiles/utiles.h>
#include <panel/panel.h>
#include <panel/kernel.h>
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

#define PANEL_PATH "../panel"
#define MAXEVENTS 64

/* Macros de respuesta MSP. */
#define MSP_RESERVE_FAILURE(s) (s == FULL_MEMORY || s == INVALID_SEG_SIZE || s == MAX_SEG_NUM_REACHED)
#define MSP_RESERVE_SUCCESS(s) (s == OK_CREATE)
#define MSP_WRITE_FAILURE(s) (s == INVALID_DIR || s == SEGMENTATION_FAULT)
#define MSP_WRITE_SUCCESS(s) (s == OK_WRITE)



typedef enum { THREAD_ID, CONSOLE_ID, CPU_ID } t_unique_id;

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
 * Nos devuelve tres tipos de id unicos segun parametro.
 */
uint32_t get_unique_id(t_unique_id id);

/*
 * Se encarga de sacar de las listas a los CPUs y Consolas salientes.
 * En caso de ser una CPU con el TCB Kernel finaliza el programa.
 */
int remove_from_lists(uint32_t sock_fd);


/* Funciones auxiliares. */
int get_puerto(void);
char *get_ip_msp(void);
int get_puerto_msp(void);
int get_quantum(void);
int get_stack_size(void);
char *get_syscalls(void);
static int make_socket_non_blocking(int sfd);


/* Archivo de configuracion. */
t_config *config;


/* Listas. */
t_list *process_list;
t_list *console_list;
t_list *cpu_list;
t_list *resource_list;
t_list *join_list;


/* Colas. */
t_queue *loader_queue;
t_queue *planificador_queue;
t_queue *syscall_queue;


/* Mutex. */
pthread_mutex_t loader_mutex;
pthread_mutex_t planificador_mutex;


/* Semaphores. */
sem_t sem_loader;
sem_t sem_planificador;


/* Hilos. */
pthread_t loader_th;
pthread_t planificador_th;


/* File descriptors. */
int msp_fd;

#endif
