#ifndef LOADER_H
#define LOADER_H

#include "kernel.h"


typedef struct {
	uint32_t console_id;
	uint32_t pid;
	uint32_t sock_fd;
} t_console;


/* Funciones loader. */
void *loader(void *arg);
t_hilo *ult_tcb(void);
t_console *new_console(uint32_t pid, uint32_t sock_fd);

#endif
