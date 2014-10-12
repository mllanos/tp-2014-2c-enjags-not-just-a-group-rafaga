#ifndef LOADER_H
#define LOADER_H

#include "kernel.h"


typedef struct {
	uint32_t console_id;
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
t_hilo *ult_tcb(void);

/*
 * Crea una nueva estructura de consola.
 */
t_console *new_console(uint32_t pid, uint32_t sock_fd);

#endif
