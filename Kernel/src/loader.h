#ifndef LOADER_H
#define LOADER_H

#include "kernel.h"


typedef struct {
	uint32_t pid;
	uint32_t sock_fd;
} t_console;

/* Funciones loader. */

/*
 * Hilo principal del Loader.
 */
void *loader(void *arg);

/*
 * Crea un nuevo TCB de usuario.
 */
t_hilo *ult_tcb(uint32_t pid);

/*
 * Crea una nueva estructura de consola.
 */
t_console *new_console(uint32_t sock_fd);

/*
 * Funciones auxiliares de consola.
 */
t_console *find_console_by_pid(uint32_t pid);
t_console *remove_console_by_sock_fd(uint32_t sock_fd);
void inform_consoles_without_active_processes(void);

#endif
