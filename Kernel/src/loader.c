#include "loader.h"

void *loader(void *arg)
{
	while(1) {
		sem_wait(&sem_loader);

		pthread_mutex_lock(&loader_mutex);
		t_msg *recibido = queue_pop(loader_queue);
		pthread_mutex_unlock(&loader_mutex);

		int *sockfd = malloc(sizeof(int));
		sockfd = memcpy(sockfd, recibido->argv, sizeof(int));
	
		t_hilo *new_tcb = reservar_memoria(ult_tcb(), recibido);

		if(new_tcb == NULL) {
			/* Couldn't allocate memory. */
			t_msg *msg = string_message(KILL_CONSOLE, "No se pudo reservar memoria en la MSP.", 0);
			enviar_mensaje(*sockfd, msg);
			destroy_message(msg);
		} else {
			/* Initialize registers and push process to the new queue. */
			int i;
			for(i = 0; i < 5; i++)
				new_tcb->registros[i] = 0;
			dictionary_put(sockfd_dict, string_itoa(new_tcb->pid), sockfd); /* KEY: PID, VALUE: SOCKFD. */
			queue_push(new_queue, new_tcb);
		}

	}
}


t_hilo *ult_tcb(void)
{
	t_hilo *new = malloc(sizeof(*new));	
	new->pid = get_unique_id();
	new->tid = new->pid;
	new->kernel_mode = false;

	return new;
}

