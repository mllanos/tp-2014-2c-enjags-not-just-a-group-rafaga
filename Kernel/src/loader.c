#include "loader.h"

void *loader(void *arg)
{
	while (1) {
		if(sem_wait(&sem_loader) == -1) {
			perror("loader");
			exit(EXIT_FAILURE);
		}

		if(!thread_alive)
			return NULL;

		pthread_mutex_lock(&loader_mutex);
		t_msg *recibido = queue_pop(loader_queue);
		pthread_mutex_unlock(&loader_mutex);

		t_console *console = new_console(recibido->argv[0]);

		pthread_mutex_lock(&console_list_mutex);
		list_add(console_list, console);
		pthread_mutex_unlock(&console_list_mutex);

		conexion_consola(console->pid);
		log_trace(logger_old, "[NEW_CONNECTION @ LOADER]: (CONSOLE_ID %u, CONSOLE_SOCK %u).", console->pid, console->sock_fd);

		t_hilo *new_tcb = reservar_memoria(ult_tcb(console->pid), recibido);
		if (new_tcb == NULL) {
			log_warning(logger_old, "No se pudo cargar a memoria el hilo principal de la Consola %u.", console->pid);

			t_msg *msg = string_message(KILL_CONSOLE, "Finalizando consola. Motivo: no hay espacio suficiente en MSP.", 0);
			if(enviar_mensaje(console->sock_fd, msg) == -1) {
				log_warning(logger_old, "Se perdio la conexion con la consola %u.", console->pid);
				remove_console_by_sock_fd(console->sock_fd);
			}
			destroy_message(msg);
		} else {
			pthread_mutex_lock(&process_list_mutex);
			list_add(process_list, new_tcb);
			pthread_mutex_unlock(&process_list_mutex);

			log_trace(logger_old, "[LOADER]: (PID %u, TID %u) => NEW.", new_tcb->pid, new_tcb->tid);

			sem_post(&sem_planificador);
		}
	}
	
	return NULL;
}


t_hilo *ult_tcb(uint32_t pid)
{
	t_hilo *new = malloc(sizeof *new);
	memset(new, 0, sizeof *new);
	new->pid = pid;
	new->kernel_mode = false;
	new->cola = NEW;

	return new;
}


t_console *new_console(uint32_t sock_fd)
{
	t_console *new = malloc(sizeof *new);
	new->pid = get_unique_id(CONSOLE_ID);
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
			return a_tcb->pid == a_cnsl->pid && a_tcb->kernel_mode == false;
		}

		if (list_count_satisfying(process_list, (void *) _find_active_by_pid) == 0) {
			t_msg *msg = string_message(KILL_CONSOLE, "Finalizando consola. Motivo: fin de ejecucion.", 0);
			if(enviar_mensaje(a_cnsl->sock_fd, msg) == -1)
				log_warning(logger_old, "Se perdio la conexion con la Consola %u.", a_cnsl->pid);
			destroy_message(msg);
		}
	}

	pthread_mutex_lock(&console_list_mutex);
	list_iterate(console_list, (void *) _inform_consoles_without_active_processes);
	pthread_mutex_unlock(&console_list_mutex);
}
