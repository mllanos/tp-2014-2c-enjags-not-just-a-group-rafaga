#include "loader.h"

void *loader(void *arg)
{
	while (1) {
		sem_wait(&sem_loader);

		pthread_mutex_lock(&loader_mutex);
		t_msg *recibido = queue_pop(loader_queue);
		pthread_mutex_unlock(&loader_mutex);

		uint32_t sock_fd = recibido->argv[0];

		t_hilo *new_tcb = reservar_memoria(ult_tcb(), recibido);
		if (new_tcb == NULL) {
			/* Couldn't allocate memory. */
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

			/* Add console to list. */
			t_console *console = new_console(new_tcb->pid, sock_fd);
			list_add(console_list, console);

			/* Log console connection. */
			conexion_consola(console->console_id);

			/* Avisamos a planificador que hay una nueva consola. */
			sem_post(&sem_planificador);
		}
	}
}


t_hilo *ult_tcb(void)
{
	t_hilo *new = malloc(sizeof *new);	
	new->pid = get_unique_id(THREAD_ID);
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