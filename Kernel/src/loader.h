#ifndef LOADER_H
#define LOADER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <commons/config.h>
#include <utiles/utiles.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <panel/panel.h>

void *loader(void *arg);
t_hilo *new_tcb(t_hilo *tcb);
t_hilo *reservar_memoria(char *beso_data);
uint32_t get_unique_id(void);
t_process *crear_proceso(int sockfd, t_hilo *tcb);
void eliminar_proceso(t_process *proc);

#endif
