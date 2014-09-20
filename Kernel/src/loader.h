#ifndef LOADER_H
#define LOADER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <utiles/utiles.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <panel/panel.h>
#include "kernel.h"

void *loader(void *arg);
t_hilo *ult_tcb(t_hilo *tcb);
t_hilo *reservar_memoria(t_hilo *tcb, char *buf);
uint32_t get_unique_id(void);
void interpret_message(int sockfd, t_msg *recibido);

#endif
