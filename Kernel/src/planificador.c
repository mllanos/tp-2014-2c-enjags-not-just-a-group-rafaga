#include "planificador.h"

void *planificador(void *arg)
{
	while (1) {
		t_msg *recibido;
		sem_wait(&sem_planificador);

		hilos(process_list);

		if (!queue_is_empty(planificador_queue)) {
			/* Mensaje entrante. */
			pthread_mutex_lock(&planificador_mutex);
			recibido = queue_pop(planificador_queue);
			pthread_mutex_unlock(&planificador_mutex);
		} else {
			/* Aviso de loader por Consola nueva. */
			bprr_algorithm();
			assign_processes();
			continue;
		}

		putmsg(recibido);

		switch (recibido->header.id) {	
			case CPU_CONNECT:
				cpu_add(recibido->argv[0]);
				break;
			case CPU_TCB:
				cpu_queue(recibido->argv[0]);
				bprr_algorithm();
				assign_processes();
				break;
			case RETURN_TCB:
				return_process(recibido->argv[0], retrieve_tcb(recibido));
				assign_processes();
				break;
			case FINISHED_THREAD:
				finish_process(recibido->argv[0], retrieve_tcb(recibido));
				assign_processes();
				break;
			case CPU_INTERRUPT:
				syscall_start(recibido->argv[0], retrieve_tcb(recibido));
				break;
			case NUMERIC_INPUT:
			case STRING_INPUT:
			case STRING_OUTPUT:
				standard_io(recibido);
				break;
			case REPLY_NUMERIC_INPUT:
				return_numeric_input(recibido->argv[0], recibido->argv[1]);
				break;
			case REPLY_STRING_INPUT:
				return_string_input(recibido->argv[0], recibido->stream);
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

void cpu_queue(uint32_t sock_fd)
{
	bool _find_by_sock_fd(t_cpu *a_cpu) {
		return a_cpu->sock_fd == sock_fd;
	}

	t_cpu *cpu = list_find(cpu_list, (void *) _find_by_sock_fd);

	queue_push(request_queue, cpu);
}


void bprr_algorithm(void)
{
	/* Muevo los procesos de NEW a READY. */

	void _new_to_ready(t_hilo *a_tcb) {
		if (a_tcb->cola == NEW)
			a_tcb->cola = READY;
	}

	list_iterate(process_list, (void *) _new_to_ready);

	/* Desbloquear los hilos que se hayan unido a un hilo EXIT. */

	void _desbloquear_join(t_hilo *a_tcb) {
		if (a_tcb->cola == EXIT) {
			int *tid_caller = dictionary_get(join_dict, string_from_format("%u", a_tcb->tid));
			if (tid_caller != NULL) {
				bool _process_by_tid(t_hilo *b_tcb) {
					return b_tcb->tid == *tid_caller;
				}

				t_hilo *desbloqueado = list_find(process_list, (void *) _process_by_tid);
				desbloqueado->cola = READY;
			}
		}
	}

	list_iterate(process_list, (void *) _desbloquear_join);

	/* Pongo en EXIT a los hijos de un hilo EXIT. */

	void _matar_hijos(t_hilo *a_tcb) {
		if (a_tcb->pid == a_tcb->tid && a_tcb->cola == EXIT) {
			/* TCB es padre y esta en EXIT. Buscar a sus hijos. */
			void _setear_exit(t_hilo *b_tcb) {
				if (b_tcb->pid == a_tcb->pid)
					b_tcb->cola = EXIT;
			}

			list_iterate(process_list, (void *) _setear_exit);
		}
	}

	list_iterate(process_list, (void *) _matar_hijos);

	/* Liberar los segmentos de los procesos EXIT. */

	void _destroy_segments_on_exit(t_hilo *a_tcb) {
		if (a_tcb->cola == EXIT) {
			t_msg *destroy_code = argv_message(DESTROY_SEGMENT, 2, a_tcb->tid, a_tcb->segmento_codigo);
			t_msg *destroy_stack = argv_message(DESTROY_SEGMENT, 2, a_tcb->tid, a_tcb->base_stack);
			if (a_tcb->tid == a_tcb->pid) {
				enviar_mensaje(msp_fd, destroy_code);
				destroy_message(recibir_mensaje(msp_fd));
			}
			enviar_mensaje(msp_fd, destroy_stack);
			destroy_message(recibir_mensaje(msp_fd));

			destroy_message(destroy_code);
			destroy_message(destroy_stack);
		}
	}

	list_iterate(process_list, (void *) _destroy_segments_on_exit);

	/* Sacar de la lista a los procesos EXIT. */

	bool _on_exit(t_hilo *a_tcb) {
		return a_tcb->cola == EXIT;
	}

	list_remove_and_destroy_by_condition(process_list, (void *) _on_exit, (void *) free);

	/* Ordeno los procesos por kernel_mode si son READY, sino por cola. */

	bool _sort_bprr(t_hilo *a_tcb, t_hilo *b_tcb) {
		if (a_tcb->cola == READY && b_tcb->cola == READY)
			return a_tcb->kernel_mode > b_tcb->kernel_mode;
		return a_tcb->cola < b_tcb->cola;
	}

	list_sort(process_list, (void *) _sort_bprr);
}


void assign_processes(void)
{
	while(!queue_is_empty(request_queue)) {
		/* Buscamos el primer proceso READY si es que existe. */
		bool _find_by_ready(t_hilo *a_tcb) {
			return a_tcb->cola == READY;
		}

		t_hilo *tcb = list_find(process_list, (void *) _find_by_ready);
		if (tcb == NULL) 
			return;

		print_tcb(tcb);
		tcb->cola = EXEC;

		t_cpu *cpu = queue_pop(request_queue);

		t_msg *msg = tcb_message(NEXT_TCB, tcb, 1, get_quantum());

		enviar_mensaje(cpu->sock_fd, msg);

		destroy_message(msg);

		/* Seteamos CPU a ocupada y actualizamos sus datos. */
		cpu->pid = tcb->pid;
		cpu->tid = tcb->tid;
		cpu->disponible = false;
		cpu->kernel_mode = tcb->kernel_mode;
	}
}


void return_process(uint32_t sock_fd, t_hilo *tcb)
{
	if (tcb->kernel_mode == false) { 
		/* Recibido hilo comun, actualizamos el tcb recibido y lo encolamos a READY si es que existe. */
		bool _find_by_tid(t_hilo *a_tcb) {
			return a_tcb->tid == tcb->tid;
		}

		t_hilo *to_update = list_find(process_list, (void *) _find_by_tid);

		if (to_update != NULL) {
			memcpy(to_update, tcb, sizeof *tcb);
			to_update->cola = READY;
		}
	} else { 
		/* Recibido hilo de Kernel, copiamos registros al proceso bloqueado por syscalls y lo encolamos a READY. */
		t_syscall *syscall = queue_pop(syscall_queue);
		memcpy(syscall->blocked->registros, tcb->registros, sizeof(int32_t) * 5);
		syscall->blocked->cola = READY;

		/* Buscamos el hilo de Kernel. */

		bool _find_by_kernel_mode(t_hilo *a_tcb) {
			return a_tcb->kernel_mode;
		}

		t_hilo *tcb_klt = list_find(process_list, (void *) _find_by_kernel_mode);

		if(queue_is_empty(syscall_queue)) {
			/* Todavia hay syscalls que atender. Cargar el proximo proceso bloqueado por syscalls. */
			t_syscall *to_load = queue_peek(syscall_queue);
			memcpy(tcb_klt->registros, to_load->blocked->registros, sizeof(int32_t) * 5);
			tcb_klt->pid = tcb->pid;
			tcb_klt->tid = tcb->tid;
			tcb_klt->puntero_instruccion = to_load->call_dir;
		} else {
			/* Ya no hay mas syscalls. Encolar el hilo de Kernel a BLOCK. */
			tcb_klt->cola = BLOCK;
		}


	}

	/* Seteamos la CPU a disponible. */
	bool _find_by_sock_fd(t_cpu *a_cpu) {
		return a_cpu->sock_fd == sock_fd;
	}

	t_cpu *cpu = list_find(cpu_list, (void *) _find_by_sock_fd);
	cpu->disponible = true;

	free(tcb);
}


void finish_process(uint32_t sock_fd, t_hilo *tcb)
{
	bool _find_by_tid(t_hilo *a_tcb) {
		return a_tcb->tid == tcb->tid;
	}

	t_hilo *finished = list_find(process_list, (void *) _find_by_tid);
	if (finished != NULL) {
		/* Si existe actualizamos TCB y encolamos a EXIT. */
		memcpy(finished, tcb, sizeof *tcb);
		finished->cola = EXIT;
	}

	free(tcb);
}

void syscall_start(uint32_t call_dir, t_hilo *tcb)
{
	/* Seteamos TCB a BLOCK y lo encolamos en la cola de syscalls. */

	bool _find_by_tid(t_hilo *a_tcb) {
		return a_tcb->tid == tcb->tid;
	}

	t_hilo *blocked = list_find(process_list, (void *) _find_by_tid);
	memcpy(blocked, tcb, sizeof *tcb);
	blocked->cola = BLOCK;

	t_syscall *new_syscall = malloc(sizeof *new_syscall);
	new_syscall->call_dir = call_dir;
	new_syscall->blocked = blocked;

	queue_push(syscall_queue, new_syscall);

	/* Buscamos el hilo de Kernel. */

	bool _find_kernel_tcb(t_hilo *a_tcb) {
		return a_tcb->kernel_mode;
	}

	t_hilo *tcb_klt = list_find(process_list, (void *) _find_kernel_tcb);

	if(tcb_klt->cola == BLOCK) {
		/* Hilo de Kernel libre, cargar datos de tcb y encolar a READY. */
		memcpy(tcb_klt->registros, tcb->registros, sizeof(int32_t) * 5);
		tcb_klt->pid = tcb->pid;
		tcb_klt->tid = tcb->tid;
		tcb_klt->puntero_instruccion = call_dir;
		tcb_klt->cola = READY;
	}

	free(tcb);
}


void standard_io(t_msg *msg)
{
	uint32_t console_pid = msg->argv[1];

	bool _find_by_pid(t_console *a_cnsl) {
		return a_cnsl->pid == console_pid;
	}

	t_console *console = list_find(console_list, (void *) _find_by_pid);

	/* Nota: msg contiene el sock_fd del CPU para terminar la operacion en return_x_input(). */
	enviar_mensaje(console->sock_fd, msg);
}


void return_numeric_input(uint32_t cpu_sock_fd, int32_t number)
{
	t_msg *msg = argv_message(REPLY_NUMERIC_INPUT, 1, number);
	enviar_mensaje(cpu_sock_fd, msg);
	destroy_message(msg);
}


void return_string_input(uint32_t cpu_sock_fd, char *stream)
{
	t_msg *msg = string_message(REPLY_STRING_INPUT, stream, 0);
	enviar_mensaje(cpu_sock_fd, msg);
	destroy_message(msg);
}


void create_thread(t_hilo *padre)
{
	uint32_t new_tid = get_unique_id(THREAD_ID);

	t_msg *create_stack = argv_message(CREATE_SEGMENT, 2, new_tid, get_stack_size());

	enviar_mensaje(msp_fd, create_stack);

	t_msg *status_stack = recibir_mensaje(msp_fd);

	if (MSP_RESERVE_SUCCESS(status_stack->header.id)) { 
		/* Memoria reservada, crear nuevo hilo y encolar a READY. */
		t_hilo *new_tcb = malloc(sizeof *new_tcb);
		new_tcb->pid = padre->pid;
		new_tcb->tid = new_tid;
		new_tcb->kernel_mode = false;
		new_tcb->segmento_codigo = padre->segmento_codigo;
		new_tcb->segmento_codigo_size = padre->segmento_codigo_size;
		new_tcb->puntero_instruccion = 0;
		new_tcb->base_stack = status_stack->argv[0];
		new_tcb->cursor_stack = new_tcb->base_stack;
		new_tcb->cola = READY;
		int i;
		for(i = 0; i < 5; i++)
			new_tcb->registros[i] = 0;

		list_add(process_list, new_tcb);
	} else if (MSP_RESERVE_FAILURE(status_stack->header.id)) { 
		/* No hay suficiente memoria, avisar a Consola. */

		bool _find_by_pid(t_console *a_cnsl) {
			return a_cnsl->pid == padre->pid;
		}

		t_console *console = list_find(console_list, (void *) _find_by_pid);

		t_msg *msg = string_message(KILL_CONSOLE, "No se pudo reservar memoria en la MSP para un hilo nuevo.", 0);
		enviar_mensaje(console->sock_fd, msg);
		destroy_message(msg);
	} else {
		errno = EBADMSG;
		perror("create_thread");
		exit(EXIT_FAILURE);	
	}

	free(padre);
	destroy_message(create_stack);
	destroy_message(status_stack);
}


void join_thread(uint32_t tid_caller, uint32_t tid_towait)
{
	uint32_t *caller = malloc(sizeof *caller);
	*caller = tid_caller;
	dictionary_put(join_dict, string_from_format("%u", tid_towait), caller);

	bool _find_by_tid(t_hilo *a_tcb) {
		return a_tcb->tid == tid_caller;
	}

	t_hilo *tcb_caller = list_find(process_list, (void *)_find_by_tid);

	tcb_caller->cola = BLOCK;
}


void block_thread(uint32_t resource, t_hilo *tcb)
{
	char *resource_id = string_from_format("%u", resource);

	t_queue *rsc_queue = dictionary_get(resource_dict, resource_id);
	if (rsc_queue == NULL) { 
		/* Crear nuevo recurso. */
		rsc_queue = queue_create();
		dictionary_put(resource_dict, resource_id, rsc_queue);
	}

	bool _find_by_tid(t_hilo *a_tcb) {
		return a_tcb->tid == tcb->tid;
	}

	t_hilo *to_block = list_find(process_list, (void *) _find_by_tid);
	to_block->cola = BLOCK;
	queue_push(rsc_queue, to_block);
}


void wake_thread(uint32_t resource)
{
	char *resource_id = string_from_format("%u", resource);

	t_queue *rsc_queue = dictionary_get(resource_dict, resource_id);

	t_hilo *woken = queue_pop(rsc_queue);
	woken->cola = READY;
}
