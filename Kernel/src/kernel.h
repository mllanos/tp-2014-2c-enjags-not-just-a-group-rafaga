#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <string.h>
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
#include "loader.h"
#include "planificador.h"

#define PANEL_PATH "../panel"

int msp_fd;

void initialize(char *config_path);
void finalize(void);
void boot_kernel(void);
void msp_connect(void);
t_hilo *klt_tcb(void);

/* Funciones de acceso a config. */
int get_puerto(void);
char *get_ip_msp(void);
int get_puerto_msp(void);
int get_quantum(void);
int get_stack_size(void);
char *get_syscalls(void);

/* Archivo de configuracion. */
t_config *config;

/* Diccionario de sockfd - pid. */
t_dictionary *sockfd_dict;

/* Colas de estado de procesos. */
t_queue *new_queue;
t_queue *ready_queue;
t_queue *exec_queue;
t_queue *block_queue;
t_queue *exit_queue;

#endif
