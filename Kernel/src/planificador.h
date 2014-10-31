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
	int call_dir;
	t_hilo *blocked;
} t_syscall;

/*
typedef struct {
	uint32_t id_resource;
	t_queue *queue;
} t_resource;


typedef struct {
	uint32_t tid_waiter;
	uint32_t tid_joined;
} t_join;
*/

/* Funciones planificador. */

/*
 * Hilo principal del Planificador.
 */
void *planificador(void *arg);

/*
 * Algoritmo Boolean Priority Round Robin
 */
void bprr_algorithm(void);

/*
 * Agregamos la nueva CPU a la lista de CPUs.
 */
void cpu_add(uint32_t sock_fd);

/*
 * Encolamos el pedido de TCB de un CPU.
 */
void cpu_queue(uint32_t sock_fd);

/*
 * Enviamos los TCB READY a los CPUs que lo hayan pedido.
 */
void assign_processes(void);

/*
 * Agregamos el proceso a READY y desalojamos el CPU. 
 */
void return_process(uint32_t sock_fd, t_hilo *tcb);

/*
 * Agregamos el proceso a EXIT y desalojamos el CPU. 
 */
void finish_process(uint32_t sock_fd, t_hilo *tcb);

/*
 * Bloqueamos el tcb y ponemos el hilo Kernel en READY.
 */
void syscall_start(uint32_t call_dir, t_hilo *tcb);

/*
 * Envia a Consola un pedido de IO.
 */
void standard_io(t_msg *msg);

/*
 * Envia a CPU el string input de su Consola asignada.
 */
void return_string_input(uint32_t cpu_sock_fd, char *stream);

/*
 * Envia a CPU el numeric input de su Consola asignada.
 */
void return_numeric_input(uint32_t cpu_sock_fd, int32_t number);

/*
 * Creamos un nuevo TCB de iguall pid que el padre, reservamos el segmento y lo encolamos a READY.
 */
void create_thread(t_hilo *padre);

/*
 * Bloquea al TCB caller hasta que el towait termine.
 */
void join_thread(uint32_t tid_caller, uint32_t tid_towait);

/*
 * Bloquea un TCB en base a un recurso.
 */
void block_thread(uint32_t resource, t_hilo *tcb);

/*
 * Libera al primer TCB en la cola de bloqueados del recurso.
 */
void wake_thread(uint32_t resource);

#endif
