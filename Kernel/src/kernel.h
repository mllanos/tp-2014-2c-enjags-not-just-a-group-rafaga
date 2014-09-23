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
#include <commons/collections/dictionary.h>
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
#include "loader.h"
#include "planificador.h"

#define PANEL_PATH "../panel"


/* Estructuras de datos. */
typedef struct {
	int console_fd;
	t_hilo *main_thread;
	t_list *threads;
	int max_thread;
} t_process;


/* Funciones Kernel. */
void initialize(char *config_path);
void finalize(void);
void boot_kernel(void);
void msp_connect(void);
void receive_messages(void);
void interpret_message(int sockfd, t_msg *recibido);
t_hilo *klt_tcb(void);
t_hilo *reservar_memoria(t_hilo *tcb, char *buf);
uint32_t get_unique_id(void);


/* Funciones de acceso a config. */
int get_puerto(void);
char *get_ip_msp(void);
int get_puerto_msp(void);
int get_quantum(void);
int get_stack_size(void);
char *get_syscalls(void);


/* Archivo de configuracion. */
t_config *config;


/* Diccionario de sockfd con acceso por pid. */
t_dictionary *sockfd_dict;


/* Colas de estado de procesos. */
t_queue *new_queue;
t_queue *ready_queue;
t_queue *exec_queue;
t_queue *block_queue;
t_queue *exit_queue;


/* Colas de mensajes a atender por los subsistemas. */
t_queue *loader_queue;
t_queue *planificador_queue;

/* Lista de CPU. */
t_list *cpu_list;


/* Mutex. */
pthread_mutex_t new_mutex;
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
