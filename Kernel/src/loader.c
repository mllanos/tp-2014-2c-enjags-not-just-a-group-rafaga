#include "loader.h"

void *loader(void *arg)
{
	while(1) {
		sem_wait(&sem_loader);

		pthread_mutex_lock(&loader_mutex);
		char *buffer = buffer = queue_pop(loader_queue);
		pthread_mutex_unlock(&loader_mutex);

		char **args = string_split(buffer, "|");
	
		t_hilo *new_tcb = reservar_memoria(ult_tcb(), args[1]);

		if(new_tcb == NULL) {

			/* Couldn't allocate memory. */
			t_msg *e_msg = new_message(NOT_ENOUGH_MEMORY, string_duplicate("No se pudo reservar memoria en la MSP."));
			enviar_mensaje(atoi(args[0]), e_msg);
			destroy_message(e_msg);
		} else {

			/* Initialize registers and push process to the new queue. */
			int i;
			for(i = 0; i < 5; i++)
				new_tcb->registros[i] = 0;
			dictionary_put(sockfd_dict, string_itoa(new_tcb->pid), string_duplicate(args[1]));
			queue_push(new_queue, new_tcb);
		}

		free(buffer);
		free(args[0]);
		free(args[1]);
		free(args);
	}
}


t_hilo *ult_tcb(void)
{
	t_hilo *new = malloc(sizeof(*new));	
	new->pid = get_unique_id();
	new->tid = 0;
	new->kernel_mode = false;

	return new;
}