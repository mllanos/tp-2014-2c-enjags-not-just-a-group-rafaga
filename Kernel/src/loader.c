#include "loader.h"

void *loader(void *arg)
{
	while (1) {
		sem_wait(&sem_loader);
		puts("AAAAAAAAAAAAAAAAAAAAAAAAAAAA");
		pthread_mutex_lock(&loader_mutex);
		t_msg *recibido = queue_pop(loader_queue);
		puts("aaaaaaaaaaaaaaaaaaaaaaaaaa");
		pthread_mutex_unlock(&loader_mutex);

		uint32_t sock_fd = recibido->argv[0];
		uint32_t new_pid = get_unique_id(THREAD_ID);

		puts("ASIGNADO.");
		sleep(3000);

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
			int i;
			for (i = 0; i < 5; i++)
				new_tcb->registros[i] = 0;
			new_tcb->cola = NEW;
			list_add(process_list, new_tcb);

			log_trace(logger, "Encolando el hilo principal (TID %u) de la Consola %u a NEW.", new_tcb->tid, console->console_id);

			/* Add console to list. */
			list_add(console_list, console);

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
