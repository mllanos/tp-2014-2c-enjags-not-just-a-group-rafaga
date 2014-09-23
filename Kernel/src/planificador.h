#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include "kernel.h"

/* Estructuras de datos. */
typedef struct {
	int cpu_fd;
	t_hilo *active_tcb;
} t_cpu;


/* Funciones planificador. */
void *planificador(void *arg);
void cpu_add(int sockfd);
void console_notify(int sockfd);
t_cpu *cpu_remove(int sockfd);
t_hilo *get_tcb(char *tcb_stream);
void disconnect_cpu(int sockfd);
void create_thread(int sockfd, char *tcb_stream);

#endif
