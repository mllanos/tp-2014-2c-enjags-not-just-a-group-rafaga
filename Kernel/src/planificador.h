#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include "kernel.h"


typedef struct {
	uint32_t cpu_id;
	uint32_t sock_fd;
	uint32_t pid;
	uint32_t tid;
	bool disponible;
	bool kernel_mode;
} t_cpu;


typedef struct {
	uint32_t id_resource;
	t_queue *queue;
} t_resource;


/* Funciones planificador. */
void *planificador(void *arg);
void cpu_add(uint32_t sock_fd);
void assign_process(uint32_t sock_fd);
void syscall_start(uint32_t call_dir, t_hilo *tcb);
void standard_input(t_msg *msg);
void standard_output(t_msg *msg);
void create_thread(t_hilo *tcb);
void join_thread(uint32_t tid_caller, uint32_t tid_waiter);
void block_thread(uint32_t resource, t_hilo *tcb);
void wake_thread(uint32_t resource);

#endif
