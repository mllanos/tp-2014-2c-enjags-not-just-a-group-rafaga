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


typedef enum { THREAD_ID, CONSOLE_ID, CPU_ID } t_unique_id;


/* Funciones Kernel. */
void initialize(char *config_path);
void finalize(void);
void boot_kernel(void);
void receive_messages(void);
void interpret_message(int sock_fd, t_msg *recibido);
t_hilo *reservar_memoria(t_hilo *tcb, t_msg *msg);
uint32_t get_unique_id(t_unique_id id);
int remove_from_lists(uint32_t sock_fd);


/* Funciones de acceso a config. */
int get_puerto(void);
char *get_ip_msp(void);
int get_puerto_msp(void);
int get_quantum(void);
int get_stack_size(void);
char *get_syscalls(void);


/* Archivo de configuracion. */
t_config *config;


/* Listas. */
t_list *process_list;
t_list *console_list;
t_list *cpu_list;
t_list *resource_list;


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
