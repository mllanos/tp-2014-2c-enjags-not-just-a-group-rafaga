#include "planificador.h"

void *planificador(void *arg)
{
	while (1) {
		t_msg *recibido;
		sem_wait(&sem_planificador);

		//hilos(process_list);

		if (!queue_is_empty(planificador_queue)) {
			pthread_mutex_lock(&planificador_mutex);
			recibido = queue_pop(planificador_queue);
			pthread_mutex_unlock(&planificador_mutex);
		} else {
			if(!list_is_empty(cpu_list)) {
				bprr_algorithm();
				assign_processes();
			}
			continue;
		}

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
			case CPU_ABORT:
				cpu_abort(recibido->argv[0], retrieve_tcb(recibido));
				break;
			case CPU_INTERRUPT:
				syscall_start(recibido->argv[0], retrieve_tcb(recibido));
				break;
			case NUMERIC_INPUT:
				numeric_input(recibido->argv[0], recibido->argv[1]);
				break;
			case STRING_INPUT:
				string_input(recibido->argv[0], recibido->argv[1], recibido->argv[2]);
				break;
			case NUMERIC_OUTPUT:
				numeric_output(recibido->argv[0], recibido->argv[1]);
				break;
			case STRING_OUTPUT:
				string_output(recibido->argv[0], recibido->stream);
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

	return NULL;

}


void bprr_algorithm(void)
{
	log_trace(logger, "Ejecutando el algoritmo BPRR.");
	log_processes("Antes del algoritmo");	
	new_processes_to_ready();
	unlock_joined_processes();
	kill_child_processes();
	destroy_segments_on_exit();
	remove_processes_on_exit();
	sort_processes_by_bprr();
	log_processes("Despues del algoritmo");
	inform_consoles_without_active_processes();
}


void assign_processes(void)
{
	while(!queue_is_empty(request_queue)) {
		/* Buscamos el primer proceso READY si es que existe. */

		t_hilo *tcb = find_process_by_ready();
		if (tcb == NULL) 
			return;

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

		log_trace(logger, "Ocupando CPU %u. Motivo: asignando hilo (TID %u, %s).", cpu->cpu_id, tcb->tid, 
			tcb->kernel_mode ? "KLT" : "ULT");
	}
}


void cpu_add(uint32_t sock_fd)
{
	t_cpu *new_cpu = malloc(sizeof *new_cpu);
	new_cpu->cpu_id = get_unique_id(CPU_ID);
	new_cpu->sock_fd = sock_fd;
	new_cpu->pid = 0;
	new_cpu->tid = 0;
	new_cpu->disponible = true;
	new_cpu->kernel_mode = false;

	pthread_mutex_lock(&cpu_list_mutex);
	list_add(cpu_list, new_cpu);
	pthread_mutex_unlock(&cpu_list_mutex);

	conexion_cpu(new_cpu->cpu_id);
	log_trace(logger, "Nueva conexion de CPU %u.", new_cpu->cpu_id);
}

void cpu_queue(uint32_t sock_fd)
{
	t_cpu *cpu = find_cpu_by_sock_fd(sock_fd);
	queue_push(request_queue, cpu);
	log_trace(logger, "Encolando pedido de TCB de CPU %u.", cpu->cpu_id);
}


void cpu_abort(uint32_t sock_fd, t_hilo *tcb)
{
	t_cpu *cpu = find_cpu_by_sock_fd(sock_fd);
	cpu->disponible = true;

	log_trace(logger, "Liberando CPU %u. Motivo: abortando proceso (PID %u).", cpu->cpu_id, tcb->pid);

	print_tcb(tcb);
	
	/* Finalizamos la consola del proceso. */

	t_console *console = find_console_by_pid(tcb->pid);

	t_msg *msg = string_message(KILL_CONSOLE, "Finalizando consola. Motivo: abort.", 0);
	enviar_mensaje(console->sock_fd, msg);
	destroy_message(msg);

	if(tcb->kernel_mode == true) {
		/* Abortado hilo de Kernel. */
		if(queue_is_empty(syscall_queue) == false) {
			/* Todavia hay syscalls que atender. Cargar el proximo proceso bloqueado por syscalls. */
			t_syscall *to_load = queue_peek(syscall_queue);
			memcpy(klt_tcb->registros, to_load->blocked->registros, sizeof to_load->blocked->registros);
			klt_tcb->pid = to_load->blocked->pid;
			klt_tcb->tid = to_load->blocked->tid;
			klt_tcb->puntero_instruccion = to_load->call_dir;
			klt_tcb->cola = READY;
			log_trace(logger, "Cargando KLT. Motivo: atender syscalls (TID %u, puntero instruccion protegida %u).",
				to_load->blocked->pid, to_load->blocked->tid, to_load->call_dir);
		} else {
			/* Ya no hay mas syscalls. Encolar el KLT a BLOCK. */
			klt_tcb->cola = BLOCK;
			log_trace(logger, "Bloqueando el KLT.");
		}
	}

	free(tcb);
}


void return_process(uint32_t sock_fd, t_hilo *tcb)
{
	
	print_tcb(tcb);
	/* Seteamos la CPU a disponible. */
	bool _find_by_sock_fd(t_cpu *a_cpu) {
		return a_cpu->sock_fd == sock_fd;
	}

	t_cpu *cpu = list_find(cpu_list, (void *) _find_by_sock_fd);
	cpu->disponible = true;

	log_trace(logger, "Liberando CPU %u. Motivo: fin de quantum hilo (TID %u, %s).", 
		cpu->cpu_id, tcb->tid, tcb->kernel_mode ? "KLT" : "ULT");

	/* Actualizamos el tcb recibido y lo encolamos a READY si es que existe. */
	t_hilo *to_update = find_process_by_tid(tcb->tid, true);

	if (to_update != NULL) {
		log_trace(logger, "Actualizando el TCB del ULT %u.", tcb->tid);
		memcpy(to_update, tcb, sizeof *tcb);
		to_update->cola = READY;
	} else {
		log_warning(logger, "El ULT %u ya no existe.", tcb->tid);
	}
	
	free(tcb);
}


void finish_process(uint32_t sock_fd, t_hilo *tcb)
{
	print_tcb(tcb);

	/* Seteamos la CPU a disponible. */
	t_cpu *cpu = find_cpu_by_sock_fd(sock_fd);
	cpu->disponible = true;

	log_trace(logger, "Desocupando CPU %u. Motivo: fin de ejecucion hilo (TID %u, %s).", 
		cpu->cpu_id, tcb->tid, tcb->kernel_mode ? "KLT" : "ULT");


	if (tcb->kernel_mode == false) { 
		/* Recibido ULT, actualizamos el tcb recibido y lo encolamos a EXIT si es que existe. */

		t_hilo *finished = find_process_by_tid(tcb->tid, true);

		if (finished != NULL) {
			/* Si existe actualizamos TCB y encolamos a EXIT. */
			memcpy(finished, tcb, sizeof *tcb);
			finished->cola = EXIT;

			log_trace(logger, "Encolando hilo (TID %u) a EXIT.", tcb->pid, tcb->tid);
		} else 
			log_warning(logger, "El hilo (TID %u) ya no existe.", tcb->pid, tcb->tid);
	} else { 
		/* Recibido KLT, copiamos registros al proceso bloqueado por syscalls y lo encolamos a READY. */
		t_syscall *syscall = queue_pop(syscall_queue);
		memcpy(syscall->blocked->registros, tcb->registros, sizeof tcb->registros);
		syscall->blocked->cola = READY;

		log_trace(logger, "Desbloqueando ULT %u. Motivo: syscall finalizada.", tcb->tid);

		if(queue_is_empty(syscall_queue) == false) {
			/* Todavia hay syscalls que atender. Cargar el proximo proceso bloqueado por syscalls. */
			t_syscall *to_load = queue_peek(syscall_queue);
			memcpy(klt_tcb->registros, to_load->blocked->registros, sizeof(int32_t) * 5);
			klt_tcb->pid = to_load->blocked->pid;
			klt_tcb->tid = to_load->blocked->tid;
			klt_tcb->puntero_instruccion = to_load->call_dir;

			log_trace(logger, "Cargando KLT. Motivo: atender syscalls (TID %u, puntero instruccion protegida %u).",
				to_load->blocked->pid, to_load->blocked->tid, to_load->call_dir);
		} else {
			/* Ya no hay mas syscalls. Encolar el KLT a BLOCK. */
			klt_tcb->cola = BLOCK;

			log_trace(logger, "Bloqueando el KLT.");
		}
	}

	free(tcb);
}


void syscall_start(uint32_t call_dir, t_hilo *tcb)
{
	/* Seteamos TCB a BLOCK y lo encolamos en la cola de syscalls. */

	t_hilo *blocked = find_process_by_tid(tcb->tid, true);
	memcpy(blocked, tcb, sizeof *tcb);
	blocked->cola = BLOCK;

	log_trace(logger, "Bloqueando hilo %u. Motivo: system call.", blocked->tid);

	t_syscall *new_syscall = malloc(sizeof *new_syscall);
	new_syscall->call_dir = call_dir;
	new_syscall->blocked = blocked;

	queue_push(syscall_queue, new_syscall);;

	if(klt_tcb->cola == BLOCK) {
		/* KLT libre, cargar datos de tcb y encolar a READY. */
		memcpy(klt_tcb->registros, tcb->registros, sizeof(int32_t) * 5);
		klt_tcb->pid = tcb->pid;
		klt_tcb->tid = tcb->tid;
		klt_tcb->puntero_instruccion = call_dir;
		klt_tcb->cola = READY;

		log_trace(logger, "Desbloqueando y cargando KLT. Motivo: atender syscalls (TID %u, puntero instruccion protegida %u).",
			blocked->pid, blocked->tid, call_dir);
	}

	free(tcb);
}


void numeric_input(uint32_t cpu_sock_fd, uint32_t tcb_pid)
{
	t_console *console = find_console_by_pid(tcb_pid);
	t_msg *msg = argv_message(NUMERIC_INPUT, 1, cpu_sock_fd);
	enviar_mensaje(console->sock_fd, msg);
	destroy_message(msg);
}


void string_input(uint32_t cpu_sock_fd, uint32_t tcb_pid, uint32_t length)
{
	t_console *console = find_console_by_pid(tcb_pid);
	t_msg *msg = argv_message(STRING_INPUT, 2, cpu_sock_fd, length);
	enviar_mensaje(console->sock_fd, msg);
	destroy_message(msg);
}


void numeric_output(uint32_t tcb_pid, int output_number)
{
	t_console *console = find_console_by_pid(tcb_pid);
	t_msg *msg = argv_message(NUMERIC_OUTPUT, 1, output_number);
	enviar_mensaje(console->sock_fd, msg);
	destroy_message(msg);
}


void string_output(uint32_t tcb_pid, char *output_stream)
{
	t_console *console = find_console_by_pid(tcb_pid);
	t_msg *msg = string_message(STRING_OUTPUT, output_stream, 0);
	enviar_mensaje(console->sock_fd, msg);
	destroy_message(msg);
}


void return_numeric_input(uint32_t cpu_sock_fd, int number)
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

	log_trace(logger, "Creando ULT hijo %u, padre ULT %u.", new_tid, padre->tid);

	t_msg *create_stack = argv_message(CREATE_SEGMENT, 2, new_tid, get_stack_size());

	enviar_mensaje(msp_fd, create_stack);

	t_msg *status_stack = recibir_mensaje(msp_fd);

	if (MSP_RESERVE_SUCCESS(status_stack->header.id)) { 
		/* Memoria reservada, crear nuevo hilo y encolar a READY. */

		log_trace(logger, "Exito al reservar memoria para el ULT hijo %u. Encolando en READY.", new_tid);

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
		memset(new_tcb->registros, 0, sizeof new_tcb->registros);

		list_add(process_list, new_tcb);
	} else if (MSP_RESERVE_FAILURE(status_stack->header.id)) { 
		/* No hay suficiente memoria, avisar a Consola. */
		t_console *console = find_console_by_pid(padre->pid);

		log_warning(logger, "Error al reservar memoria para el ULT hijo %u.", new_tid);

		t_msg *msg = string_message(KILL_CONSOLE, "Finalizando consola. Motivo: no se pudo reservar memoria en la MSP para un hilo nuevo.", 0);
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


	t_hilo *tcb_caller = find_process_by_tid(tid_caller, true);
	tcb_caller->cola = BLOCK;

	log_trace(logger, "Bloqueando ULT %u. Motivo: join ULT %u.", tid_caller, tid_towait);
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

	/* Actualizar y encolar TCB a BLOCK. */

	t_hilo *to_block = find_process_by_tid(tcb->tid, true);
	memcpy(to_block, tcb, sizeof *tcb);
	to_block->cola = BLOCK;
	queue_push(rsc_queue, to_block);

	log_trace(logger, "Bloqueando ULT %u. Motivo: pedido recurso (ID %u).", tcb->tid, resource);

	free(tcb);
}


void wake_thread(uint32_t resource)
{
	char *resource_id = string_from_format("%u", resource);

	t_queue *rsc_queue = dictionary_get(resource_dict, resource_id);

	t_hilo *woken = queue_pop(rsc_queue);
	woken->cola = READY;

	log_trace(logger, "Desbloqueando ULT %u. Motivo: liberar recurso (ID %u).", woken->tid, resource);
}


t_cpu *remove_cpu_by_sock_fd(uint32_t sock_fd)
{
	bool _remove_cpu_by_sock_fd(t_cpu *a_cpu) { 
		return a_cpu->sock_fd == sock_fd; 
	}

	pthread_mutex_lock(&cpu_list_mutex);
	t_cpu *removed = list_remove_by_condition(cpu_list, (void *) _remove_cpu_by_sock_fd);
	pthread_mutex_unlock(&cpu_list_mutex);

	return removed;
}


t_cpu *find_cpu_by_sock_fd(uint32_t sock_fd)
{
	bool _find_cpu_by_sock_fd(t_cpu *a_cpu) {
		return a_cpu->sock_fd == sock_fd;
	}

	pthread_mutex_lock(&cpu_list_mutex);
	t_cpu *found = list_find(cpu_list, (void *) _find_cpu_by_sock_fd);
	pthread_mutex_unlock(&cpu_list_mutex);

	return found;
}


void finalize_process_by_pid(uint32_t pid)
{
	void _finalize_process_by_pid(t_hilo *a_tcb) {
		if (a_tcb->pid == pid && a_tcb->kernel_mode == false) 
			a_tcb->cola = EXIT;
	}

	pthread_mutex_lock(&process_list_mutex);
	list_iterate(process_list, (void *) _finalize_process_by_pid);
	pthread_mutex_unlock(&process_list_mutex);
}


void log_processes(char *message)
{
	char *string_proc = string_new();

	void _log_processes(t_hilo *a_tcb) {
		string_append_with_format(&string_proc, "%s{ PID: %u, TID: %u, %s, %s }", string_is_empty(string_proc) ? "" : ", ", 
			a_tcb->pid, a_tcb->tid, string_cola(a_tcb->cola), a_tcb->kernel_mode ? "KLT" : "ULT");
	}

	pthread_mutex_lock(&process_list_mutex);
	list_iterate(process_list, (void *) _log_processes);
	pthread_mutex_unlock(&process_list_mutex);

	log_trace(logger, "%s:\n\t[%s]", message, string_proc);

	free(string_proc);
}


t_hilo *find_process_by_tid(uint32_t tid, bool mutex_lock)
{
	bool _find_process_by_tid(t_hilo *b_tcb) {
		return b_tcb->tid == tid;
	}

	if (mutex_lock)
		pthread_mutex_lock(&process_list_mutex);
	t_hilo *found = list_find(process_list, (void *) _find_process_by_tid);
	if (mutex_lock)
		pthread_mutex_unlock(&process_list_mutex);

	return found;
}


void unlock_joined_processes(void)
{
	void _unlock_joined_processes(t_hilo *a_tcb) {
		if (a_tcb->cola == EXIT) {
			int *tid_caller = dictionary_get(join_dict, string_from_format("%u", a_tcb->tid));
			if (tid_caller != NULL) {
				t_hilo *desbloqueado = find_process_by_tid(*tid_caller, false);
				desbloqueado->cola = READY;

				log_trace(logger, "Desbloqueando ULT %u. Motivo: fin ULT joined (%u).",
					desbloqueado->tid, a_tcb->tid);
			}
		}
	}

	pthread_mutex_lock(&process_list_mutex);
	list_iterate(process_list, (void *) _unlock_joined_processes);
	pthread_mutex_unlock(&process_list_mutex);
}


void kill_child_processes(void)
{
	void _kill_child_processes(t_hilo *a_tcb) {
		if (a_tcb->pid == a_tcb->tid && a_tcb->cola == EXIT) {
			void _set_child_to_exit(t_hilo *b_tcb) {
				if (b_tcb->pid == a_tcb->pid && b_tcb->pid != b_tcb->tid && b_tcb->cola != EXIT) {
					b_tcb->cola = EXIT;
					log_trace(logger, "Finalizando ULT %u. Motivo: fin ULT padre.", b_tcb->tid);
				}
			}

			list_iterate(process_list, (void *) _set_child_to_exit);
		}
	}

	pthread_mutex_lock(&process_list_mutex);
	list_iterate(process_list, (void *) _kill_child_processes);
	pthread_mutex_unlock(&process_list_mutex);
}


void destroy_segments_on_exit(void)
{
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

	pthread_mutex_lock(&process_list_mutex);
	list_iterate(process_list, (void *) _destroy_segments_on_exit);
	pthread_mutex_unlock(&process_list_mutex);
}


void remove_processes_on_exit(void)
{
	bool _on_exit(t_hilo *a_tcb) {
		return a_tcb->cola == EXIT;
	}

	pthread_mutex_lock(&process_list_mutex);
	list_remove_and_destroy_by_condition(process_list, (void *) _on_exit, (void *) free);
	pthread_mutex_unlock(&process_list_mutex);
}


void new_processes_to_ready(void)
{
	void _new_processes_to_ready(t_hilo *a_tcb) {
		if (a_tcb->cola == NEW)
			a_tcb->cola = READY;
	}

	pthread_mutex_lock(&process_list_mutex);
	list_iterate(process_list, (void *) _new_processes_to_ready);
	pthread_mutex_unlock(&process_list_mutex);
}


void sort_processes_by_bprr(void)
{
	bool _sort_bprr(t_hilo *a_tcb, t_hilo *b_tcb) {
		if (a_tcb->cola == READY && b_tcb->cola == READY)
			return a_tcb->kernel_mode > b_tcb->kernel_mode;
		return a_tcb->cola < b_tcb->cola;
	}

	pthread_mutex_lock(&process_list_mutex);
	list_sort(process_list, (void *) _sort_bprr);
	pthread_mutex_unlock(&process_list_mutex);
}


t_hilo *find_process_by_ready(void)
{
	bool _find_process_by_ready(t_hilo *a_tcb) {
			return a_tcb->cola == READY;
	}

	pthread_mutex_lock(&process_list_mutex);
	t_hilo *found = list_find(process_list, (void *) _find_process_by_ready);
	pthread_mutex_unlock(&process_list_mutex);

	return found;
}
