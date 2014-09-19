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
#include "loader.h"
#include "planificador.h"

#define PANEL_PATH "../panel"

void initialize(char *config_path);

/* Archivo de configuracion. */
t_config *config;

/* Colas de estado de procesos. */
t_queue *new_queue;
t_queue *ready_queue;
t_queue *exec_queue;
t_queue *block_queue;
t_queue *exit_queue;

#endif
