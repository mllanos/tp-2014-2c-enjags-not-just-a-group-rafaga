#include "loader.h"

void *loader(void *arg)
{
	while (1) {
		sem_wait(&sem_loader);
		pthread_mutex_lock(&loader_mutex);
		t_msg *recibido = queue_pop(loader_queue);
		pthread_mutex_unlock(&loader_mutex);

		uint32_t sock_fd = recibido->argv[0];
		uint32_t new_pid = get_unique_id(THREAD_ID);

		/* New console. */
		t_console *console = new_console(new_pid, sock_fd);

		/* Log console connection. */
		//conexion_consola(console->console_id);
		log_trace(logger, "Nueva conexion de Consola %u.", console->console_id);

		t_hilo *new_tcb = reservar_memoria(ult_tcb(new_pid), recibido);
		if (new_tcb == NULL) {
			/* Couldn't allocate memory. */
			log_warning(logger, "No se pudo cargar a memoria el hilo principal de la Consola %u.", console->console_id);

			t_msg *msg = string_message(KILL_CONSOLE, "Finalizando consola. Motivo: no hay espacio suficiente en MSP.", 0);
			enviar_mensaje(sock_fd, msg);
			destroy_message(msg);
		} else {
			/* Add NEW process to list. */
			memset(new_tcb->registros, 0, sizeof new_tcb->registros);
			new_tcb->cola = NEW;
			list_add(process_list, new_tcb);

			log_trace(logger, "Encolando el hilo principal (TID %u) de la Consola %u a NEW.", new_tcb->tid, console->console_id);

			/* Add console to list. */

			pthread_mutex_lock(&console_list_mutex);
			list_add(console_list, console);
			pthread_mutex_unlock(&console_list_mutex);

			/* Avisamos a planificador que hay una nueva consola. */
			sem_post(&sem_planificador);
		}
	}
}


t_hilo *ult_tcb(uint32_t pid)
{
	t_hilo *new = malloc(sizeof *new);	
	new->pid = pid;
	new->tid = new->pid;
	new->kernel_mode = false;

	return new;
}


t_console *new_console(uint32_t pid, uint32_t sock_fd)
{
	t_console *new = malloc(sizeof *new);
	new->console_id = get_unique_id(CONSOLE_ID);
	new->pid = pid;
	new->sock_fd = sock_fd;

	return new;
}


t_console *find_console_by_pid(uint32_t pid)
{
	bool _find_by_pid(t_console *a_cnsl) {
		return a_cnsl->pid == pid;
	}

	pthread_mutex_lock(&console_list_mutex);
	t_console *found = list_find(console_list, (void *) _find_by_pid);
	pthread_mutex_unlock(&console_list_mutex);

	return found;
}

t_console *remove_console_by_sock_fd(uint32_t sock_fd)
{
	bool _remove_console_by_sock_fd(t_console *a_cnsl) { 
		return a_cnsl->sock_fd == sock_fd; 
	}

	pthread_mutex_lock(&console_list_mutex);
	t_console *removed = list_remove_by_condition(console_list, (void *) _remove_console_by_sock_fd);
	pthread_mutex_unlock(&console_list_mutex);

	return removed;
}


void inform_consoles_without_active_processes(void)
{
	void _inform_consoles_without_active_processes(t_console *a_cnsl) {
		bool _find_active_by_pid(t_hilo *a_tcb) {
			return a_tcb->pid == a_cnsl->pid;
		}

		if (list_count_satisfying(process_list, (void *) _find_active_by_pid) == 0) {
			t_msg *msg = string_message(KILL_CONSOLE, "Finalizando Consola. Motivo: fin de ejecucion.", 0);
			enviar_mensaje(a_cnsl->sock_fd, msg);
			destroy_message(msg);
		}
	}

	pthread_mutex_lock(&console_list_mutex);
	list_iterate(console_list, (void *) _inform_consoles_without_active_processes);
	pthread_mutex_unlock(&console_list_mutex);
}