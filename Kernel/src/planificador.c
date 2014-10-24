#include "planificador.h"

void *planificador(void *arg)
{
	while (1) {
		sem_wait(&sem_planificador);
		
		pthread_mutex_lock(&planificador_mutex);
		t_msg *recibido = queue_pop(planificador_queue);
		pthread_mutex_unlock(&planificador_mutex);

		switch (recibido->header.id) {
			case CPU_CONNECT:
				cpu_add(recibido->argv[0]);
				break;
			case CPU_TCB:
				bprr_algorithm();
				assign_process(recibido->argv[0]);
				break;
			case RETURN_TCB:
				return_process(recibido->argv[0], retrieve_tcb(recibido));
				break;
			//case FINISHED_THREAD:
			//	break;
			case CPU_INTERRUPT:
				syscall_start(recibido->argv[0], retrieve_tcb(recibido));
				break;
			case NUMERIC_INPUT:
			case STRING_INPUT:
			case STRING_OUTPUT:
				standard_io(recibido);
				break;
			case REPLY_INPUT:
				return_input(recibido->argv[0], recibido->stream);
				break;
			case CPU_CREA:
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


void bprr_algorithm(void)
{
	/* Muevo los procesos NEW a READY. */
	void _new_to_ready(t_hilo *a_tcb) {
		if (a_tcb->cola == NEW)
			a_tcb->cola = READY;
	}

	list_iterate(process_list, (void *) _new_to_ready);

	/* Ordeno los procesos READY por prioridades. */
	bool _ordenar_por_prioridades(t_hilo *a_tcb, t_hilo *b_tcb) {
		if (a_tcb->cola == READY && b_tcb->cola == READY)
			return a_tcb->kernel_mode <= b_tcb->kernel_mode;
		return true;
	}

	list_sort(process_list, (void *) _ordenar_por_prioridades);

	/* Desbloquear los hilos que se hayan unido a un hilo saliente. */

	void _desbloquear_join(t_hilo *a_tcb) {
		if(a_tcb->cola == EXIT) {
			bool _join_by_tid(t_join *a_join) {
				return a_join->tid_joined == a_tcb->tid;
			}

			t_join *joined = list_remove_by_condition(join_list, (void *) _join_by_tid);

			if(joined != NULL) {
				bool _process_by_tid(t_hilo *b_tcb) {
					return b_tcb->tid == joined->tid_waiter;
				}

				t_hilo *desbloqueado = list_find(process_list, (void *) _process_by_tid);
				desbloqueado->cola = READY;
			}
		}
	}

	/* Liberar los recursos de los procesos EXIT. */

	bool _liberar_recursos(t_hilo *a_tcb) {
		return a_tcb->cola == EXIT;
	}

	list_remove_and_destroy_by_condition(process_list, (void *) _liberar_recursos, (void *) free);
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
	/* Enviamos TCB con quantum. */
	bool _find_first_ready(t_hilo *a_tcb) {
		return a_tcb->cola == READY;
	}

	t_hilo *tcb = list_find(process_list, (void *) _find_first_ready);
	tcb->cola = EXEC;

	t_msg *msg = tcb_message(NEXT_TCB, tcb, 1, get_quantum());

	enviar_mensaje(sock_fd, msg);

	/* Seteamos CPU a ocupada. */
	bool _find_by_sock_fd(t_cpu *a_cpu) {
		return a_cpu->sock_fd == sock_fd;
	}

	t_cpu *cpu = list_find(cpu_list, (void *) _find_by_sock_fd);
	cpu->pid = tcb->pid;
	cpu->tid = tcb->tid;
	cpu->disponible = false;
	cpu->kernel_mode = tcb->kernel_mode;

	destroy_message(msg);
}


void return_process(uint32_t sock_fd, t_hilo *tcb)
{
	if (tcb->kernel_mode == false) { /* Recibido hilo comun. */
		/* Actualizamos el tcb recibido y lo encolamos a READY. */
		void _update_tcb(t_hilo *a_tcb) {
			if (a_tcb->tid == tcb->tid) {
				memcpy(a_tcb, tcb, sizeof *tcb);
				a_tcb->cola = READY;
			}
		}

		list_iterate(process_list, (void *) _update_tcb);
	} else { /* Recibido hilo de Kernel. */
		/* Copiamos registros al proceso bloqueado por syscalls y lo encolamos a READY. */
		t_hilo *tcb_syscall = queue_pop(syscall_queue);
		memcpy(tcb_syscall->registros, tcb->registros, sizeof(int32_t) * 5);
		tcb_syscall->cola = READY;

		void _actualizar_klt(t_hilo *a_tcb) {
			if (a_tcb->kernel_mode == true) {
				memcpy(a_tcb, tcb, sizeof *tcb);
				a_tcb->cola = BLOCK;
			}
		}

		list_iterate(process_list, (void *) _actualizar_klt);
	}

	/* Seteamos la CPU a disponible. */
	bool _find_by_sock_fd(t_cpu *a_cpu) {
		return a_cpu->sock_fd == sock_fd;
	}

	t_cpu *cpu = list_find(cpu_list, (void *) _find_by_sock_fd);
	cpu->disponible = true;

	free(tcb);
}


void syscall_start(uint32_t call_dir, t_hilo *tcb)
{
	/* Seteamos TCB a BLOCK y lo encolamos en la cola de syscalls. */

	void _block_by_tid(t_hilo *a_tcb) {
		if (a_tcb->tid == tcb->tid) {
			memcpy(a_tcb, tcb, sizeof *tcb),
			a_tcb->cola = BLOCK;
			queue_push(syscall_queue, tcb);
		}
	}

	list_iterate(process_list, (void *) _block_by_tid);

	/* Copiamos registros, pid y tid del TCB a hilo Kernel y lo encolamos a READY. */

	bool _find_kernel_tcb(t_hilo *a_tcb) {
		return a_tcb->kernel_mode == true;
	}

	t_hilo *klt_tcb = list_find(process_list, (void *) _find_kernel_tcb);
	memcpy(klt_tcb->registros, tcb->registros, sizeof(int32_t) * 5);
	klt_tcb->pid = tcb->pid;
	klt_tcb->tid = tcb->tid;
	klt_tcb->puntero_instruccion = call_dir;
	klt_tcb->cola = READY;
}


void standard_io(t_msg *msg)
{
	uint32_t console_pid = msg->argv[1];

	bool _find_by_pid(t_console *a_cnsl) {
		return a_cnsl->pid == console_pid;
	}

	t_console *console = list_find(console_list, (void *) _find_by_pid);

	/* Nota: msg contiene el sock_fd del CPU para terminar la operacion en return_input(). */
	enviar_mensaje(console->sock_fd, msg);
}


void return_input(uint32_t cpu_sock_fd, char *stream)
{
	t_msg *msg = string_message(REPLY_INPUT, stream, 0);
	enviar_mensaje(cpu_sock_fd, msg);
	destroy_message(msg);
}


void create_thread(t_hilo *padre)
{
	t_hilo *new_tcb = malloc(sizeof *new_tcb);
	memcpy(new_tcb, padre, sizeof *padre);
	new_tcb->pid = padre->pid;
	new_tcb->tid = get_unique_id(THREAD_ID);

	t_msg *r_stack = string_message(CREATE_SEGMENT, "Reservando segmento de stack para hilo nuevo.", 0, new_tcb->pid, get_stack_size());

	enviar_mensaje(msp_fd, r_stack);

	t_msg *status = recibir_mensaje(msp_fd);

	if (MSP_RESERVE_SUCCESS(status->header.id)) { /* Memoria reservada. */
		/* Encolar el hilo a READY. */
		new_tcb->base_stack = status->argv[0];
		new_tcb->cursor_stack = new_tcb->base_stack;
		new_tcb->cola = READY;
		list_add(process_list, new_tcb);
	} else if (MSP_RESERVE_FAILURE(status->header.id)) { /* No hay suficiente memoria. */
		/* Finalizar todos los hilos del proceso y avisar a consola. */

		void _finalize_by_pid(t_hilo *a_tcb) {
			if (a_tcb->pid == new_tcb->pid && a_tcb->kernel_mode == false) 
				a_tcb->cola = EXIT;
		}

		list_iterate(process_list, (void *) _finalize_by_pid);

		bool _remove_by_pid(t_console *a_cnsl) {
			return a_cnsl->pid == new_tcb->pid;
		}

		t_console *console = list_remove_by_condition(console_list, (void *) _remove_by_pid);

		t_msg *msg = string_message(KILL_CONSOLE, "No se pudo reservar memoria en la MSP para un hilo nuevo.", 0);
		enviar_mensaje(console->sock_fd, msg);
		destroy_message(msg);
		free(new_tcb);
	} else {
		errno = EBADMSG;
		perror("create_thread");
		exit(EXIT_FAILURE);	
	}

	free(padre);
	destroy_message(r_stack);
	destroy_message(status);
}


void join_thread(uint32_t tid_waiter, uint32_t tid_joined)
{
	bool _find_by_tid_w(t_hilo *a_tcb) {
		return a_tcb->tid == tid_waiter;
	}

	t_hilo *tcb_waiter = list_find(process_list, (void *) _find_by_tid_w);

	bool _find_by_tid_j(t_hilo *a_tcb) {
		return a_tcb->tid == tid_joined;
	}

	t_hilo *tcb_joined = list_find(process_list, (void *) _find_by_tid_j);

	tcb_waiter->cola = BLOCK;

	t_join *new_join = malloc(sizeof *new_join);
	new_join->tid_waiter = tid_waiter;
	new_join->tid_joined = tid_joined;

	list_add(join_list, new_join);
}


void block_thread(uint32_t resource, t_hilo *tcb)
{
	bool _find_by_id(t_resource *a_res) {
		return a_res->id_resource == resource;
	}

	t_resource *rsc = list_find(resource_list, (void *) _find_by_id);
	if (rsc == NULL) { /* Crear nuevo recurso. */
		rsc = malloc(sizeof *rsc);
		rsc->id_resource = resource;
		rsc->queue = queue_create();
		list_add(resource_list, rsc);
	}

	void _block_by_tid(t_hilo *a_tcb) {
		if (a_tcb->tid == tcb->tid) {
			a_tcb->cola = BLOCK;
			queue_push(rsc->queue, a_tcb);
		}
	}

	list_iterate(process_list, (void *) _block_by_tid);
}


void wake_thread(uint32_t resource)
{
	bool _find_by_id(t_resource *a_res) {
		return a_res->id_resource == resource;
	}

	t_resource *rsc = list_find(resource_list, (void *) _find_by_id);

	t_hilo *woken = queue_pop(rsc->queue);

	void _wake_by_tid(t_hilo *a_tcb) {
		if (a_tcb->tid == woken->tid)
			a_tcb->cola = READY;
	}

	list_iterate(process_list, (void *) _wake_by_tid);
}
