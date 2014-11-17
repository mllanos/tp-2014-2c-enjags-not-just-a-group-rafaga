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
 * Finalizamos el proceso de la CPU que aborta.
 */
void cpu_abort(uint32_t sock_fd, t_hilo *tcb);

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
 * Envia a Consola un pedido de entrada numerica.
 */
void numeric_input(uint32_t cpu_sock_fd, uint32_t tcb_pid);

/*
 * Envia a Consola un pedido de entrada de cadena.
 */
void string_input(uint32_t cpu_sock_fd, uint32_t tcb_pid, uint32_t length);

/*
 * Envia a Consola un pedido de salida numerica.
 */
void numeric_output(uint32_t tcb_pid, int output_number);

/*
 * Envia a Consola un pedido de salida de cadena.
 */
void string_output(uint32_t tcb_pid, char *output_stream);

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
void join_thread(uint32_t tid_caller, uint32_t tid_towait, uint32_t process_pid);

/*
 * Bloquea un TCB en base a un recurso.
 */
void block_thread(uint32_t resource, t_hilo *tcb);

/*
 * Libera al primer TCB en la cola de bloqueados del recurso.
 */
void wake_thread(uint32_t resource);


/* Funciones auxiliares. */
void attend_next_syscall_request(void);


/* Funciones auxiliares de CPU. */
t_cpu *remove_cpu_by_sock_fd(uint32_t sock_fd);
t_cpu *find_cpu_by_sock_fd(uint32_t sock_fd);


/* Funciones auxiliares de procesos. */
void finalize_process_by_pid(uint32_t pid);
void log_processes(char *message);
void unlock_joined_processes(void);
void kill_child_processes(void);
void destroy_segments_on_exit_or_condition(bool kill_all);
void remove_processes_on_exit(void);
void new_processes_to_ready(void);
void sort_processes_by_bprr(void);
t_hilo *find_thread_by_pid_tid(uint32_t pid, uint32_t tid, bool mutex_lock);
t_hilo *find_process_by_ready(void);

#endif
