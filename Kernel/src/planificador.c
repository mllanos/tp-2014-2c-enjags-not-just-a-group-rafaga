#include "planificador.h"

void *planificador(void *arg)
{
	while(1) {
		sem_wait(&sem_planificador);
		
		pthread_mutex_lock(&planificador_mutex);
		t_msg *recibido = queue_pop(planificador_queue);
		pthread_mutex_unlock(&planificador_mutex);

		switch(recibido->header.id) {
			case CPU_CONNECT:
				cpu_add(recibido->argv[0]);
				break;
			case CPU_TCB:
				assign_process(recibido->argv[0]);
				break;
			case CPU_INTERRUPT:
				syscall_start(recibido->argv[0], retrieve_tcb(recibido));
				break;
			case NUMERIC_INPUT:
			case STRING_INPUT:
				standard_input(recibido);
				break;
			case STRING_OUTPUT:
				standard_output(recibido);
				break;
			case CPU_THREAD:
				create_thread(retrieve_tcb(recibido));
				break;
			case CPU_JOIN:
				join_thread(recibido->argv[0], recibido->argv[1]);
				break;
			case CPU_BLOCK:
				block_thread(recibido->argv[0], retrieve_tcb(recibido));
				break;
			case CPU_WAKE:
				wake_thread(recibido->argv[0]);
				break;
			default:
				errno = EBADMSG;
				perror("planificador");
				break;
		}

		destroy_message(recibido);
	}
}





void cpu_add(uint32_t sock_fd)
{
	t_cpu *new_cpu = malloc(sizeof *new_cpu);

	new_cpu->cpu_id = get_unique_id(CPU_ID);
	new_cpu->sock_fd = sock_fd;
	new_cpu->pid = -1;
	new_cpu->tid = -1;
	new_cpu->disponible = true;
	new_cpu->kernel_mode = false;

	list_add(cpu_list, new_cpu);

	conexion_cpu(new_cpu->cpu_id);
}


void assign_process(uint32_t sock_fd)
{

}


void syscall_start(uint32_t call_dir, t_hilo *tcb)
{
	tcb->cola = BLOCK;
	queue_push(syscall_queue, tcb);

	bool _is_klt(t_hilo *tcb) {
		return tcb->kernel_mode == true;
	}

	t_hilo *klt_tcb = list_find(process_list, (void *) _is_klt);
	memcpy(klt_tcb->registros, tcb->registros, sizeof(int32_t)*5);
	klt_tcb->pid = tcb->pid;
	klt_tcb->tid = tcb->tid;
	klt_tcb->puntero_instruccion = call_dir;
	klt_tcb->cola = READY;
}


void standard_input(t_msg *msg)
{
	uint32_t cpu_sock_fd = msg->argv[0];
	uint32_t pid = msg->argv[1];

	bool _get_by_sock_fd(t_console *cnsl) {
		return cnsl->sock_fd == pid;
	}

	t_console *console = list_find(console_list, (void *) _get_by_sock_fd);

	enviar_mensaje(console->sock_fd, msg);

	t_msg *recibido = recibir_mensaje(console->sock_fd);
	enviar_mensaje(cpu_sock_fd, recibido);
}


void standard_output(t_msg *msg)
{
	uint32_t cpu_sock_fd = msg->argv[0];
	uint32_t pid = msg->argv[1];

	bool _get_by_sock_fd(t_console *cnsl) {
		return cnsl->sock_fd == pid;
	}

	t_console *console = list_find(console_list, (void *) _get_by_sock_fd);

	enviar_mensaje(console->sock_fd, msg);
}


void create_thread(t_hilo *tcb)
{
	t_hilo *new_tcb = malloc(sizeof *new_tcb);
	memcpy(new_tcb, tcb, sizeof *tcb);
	new_tcb->tid = get_unique_id(THREAD_ID);

	t_msg *r_stack = string_message(RESERVE_SEGMENT, "Reservando segmento de stack para hilo nuevo.", 0, new_tcb->pid, get_stack_size());

	enviar_mensaje(msp_fd, r_stack);

	t_msg *status = recibir_mensaje(msp_fd);

	if(status->header.id == OK_RESERVE) { /* Memoria reservada. */
		/* Setear el hilo a READY y agregarlo a la lista de procesos. */
		new_tcb->base_stack = status->argv[0];
		new_tcb->cursor_stack = new_tcb->base_stack;
		new_tcb->cola = READY;
		list_add(process_list, new_tcb);
	} else if(status->header.id == ENOMEM_RESERVE) { /* No hay suficiente memoria. */
		/* Finalizar todos los hilos del proceso y avisar a consola. */

		void _finalize_by_pid(t_hilo *tcb) {
			if(tcb->pid == new_tcb->pid && tcb->kernel_mode == false) tcb->cola = EXIT;
		}

		process_list = list_map(process_list, (void *) _finalize_by_pid);

		bool _get_by_pid(t_console *cnsl) {
			return cnsl->pid == new_tcb->pid;
		}

		t_console *console = list_remove_by_condition(console_list, (void *) _get_by_pid);

		t_msg *msg = string_message(KILL_CONSOLE, "No se pudo reservar memoria en la MSP para un hilo nuevo.", 0);
		enviar_mensaje(console->sock_fd, msg);
		destroy_message(msg);
		free(new_tcb);
	} else {
		errno = EBADMSG;
		perror("create_thread");
		exit(EXIT_FAILURE);	
	}

	free(tcb);
	destroy_message(r_stack);
	destroy_message(status);
}


void join_thread(uint32_t tid_caller, uint32_t tid_waiter)
{
	bool _get_by_tid_c(t_hilo *tcb) {
		return tcb->tid == tid_caller;
	}

	t_hilo *caller = list_find(process_list, (void *) _get_by_tid_c);

	bool _get_by_tid_w(t_hilo *tcb) {
		return tcb->tid == tid_caller;
	}

	t_hilo *waiter = list_find(process_list, (void *) _get_by_tid_w);

	caller->cola = BLOCK;
}


void block_thread(uint32_t resource, t_hilo *tcb)
{
	uint32_t tid = tcb->tid;

	void _block_by_tid(t_hilo *a_tcb) {
		if(a_tcb->tid == tid) a_tcb->cola = BLOCK;
	}

	process_list = list_map(process_list, (void *) _block_by_tid);

	bool _get_resource(t_resource *res) {
		return res->id_resource == resource;
	}

	t_resource *rsc = list_find(resource_list, (void *) _get_resource);
	if(rsc == NULL) { /* Nuevo recurso. */
		rsc = malloc(sizeof *rsc);
		rsc->id_resource = resource;
		rsc->queue = queue_create();


	}

	queue_push(rsc->queue, tcb);
	list_add(resource_list, rsc);
}


void wake_thread(uint32_t resource)
{
	bool _get_resource(t_resource *res) {
		return res->id_resource == resource;
	}

	t_resource *rsc = list_find(resource_list, (void *) _get_resource);

	t_hilo *woken = queue_pop(rsc->queue);

	woken->cola = READY;
/*
	uint32_t tid = woken->tid;

	bool _get_by_tid(t_hilo *a_tcb) {
		return a_tcb->tid == tid;
	}
	list_remove_by_condition(process_list, (void *) _get_by_tid);
*/
	list_add(process_list, woken);
}