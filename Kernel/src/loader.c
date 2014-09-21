#include "loader.h"

void *loader(void *arg)
{
	fd_set master, read_fds;

	/* Create the socket and set it up to accept connections. */
	int listener = server_socket(get_puerto());

	/* Initialize the set of active sockets. */
	FD_ZERO (&master);
	FD_SET (listener, &master);

	int fdmax = listener;

	while (1) {

		/* Block until input arrives on one or more active sockets. */
		memcpy(&read_fds, &master, sizeof(read_fds));

		if (select (fdmax + 1, &read_fds, NULL, NULL, NULL) < 0) {
			perror ("select");
			exit(EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		int i;

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET (i, &read_fds)) {
				if (i == listener) {

					/* Connection request on original socket. */
					int newfd = accept_connection(listener);
					if (newfd < 0) {
						perror ("accept");
						exit(EXIT_FAILURE);
					}
					puts("Conexion.");
					FD_SET (newfd, &master);
					fdmax = newfd > fdmax ? newfd : fdmax;
				}
				else {

					/* Data arriving on an already-connected socket. */
					t_msg *recibido = recibir_mensaje(i);
					if(recibido == NULL) {

						/* Socket closed connection. */
						puts("Desconexion.");
						close (i);
						FD_CLR (i, &master);
					} else {

						/* Socket received message. */
						puts(recibido->stream);

						interpret_message(i, recibido);

						destroy_message(recibido);
					}	
				}
			}
		}
	}
}


void interpret_message(int sockfd, t_msg *recibido)
{
	t_msg *e_msg;
	t_hilo *new_tcb;
	puts("interpret_message!");

	switch(recibido->header.id) {
		case CONSOLE_CODE:
			puts("reservar_memoria!");
			new_tcb = reservar_memoria(ult_tcb(NULL), recibido->stream);
			puts("Fin reserva");
			if(new_tcb == NULL) {

				/* Couldn't allocate memory. */
				e_msg = new_message(NOT_ENOUGH_MEMORY, "No se pudo reservar memoria en la MSP.");
				enviar_mensaje(sockfd, e_msg);
				destroy_message(e_msg);
			} else {
				puts("OK_MEMORY");
				/* Initialize registers and push process to the new queue. */
				int i;
				for(i = 0; i < 5; i++)
					new_tcb->registros[i] = 0;
				dictionary_put(sockfd_dict, string_itoa(new_tcb->pid), string_itoa(sockfd));
				queue_push(new_queue, new_tcb);
			}
			break;
		default:
			errno = EBADMSG;
			perror("interpret_message");
			exit(EXIT_FAILURE);
	}
}


uint32_t get_unique_id(void)
{
	static int x = 0;
	return ++x;
}


t_hilo *reservar_memoria(t_hilo *tcb, char *buf)
{
	int i, cont = 1;
	t_msg *message[3];
	t_msg *status[3];

	message[0] = new_message(RESERVE_CODE, string_from_format("%d:%d", tcb->pid, strlen(buf)));
	message[1] = new_message(RESERVE_STACK, string_from_format("%d:%d", tcb->pid, get_stack_size()));
	message[2] = new_message(WRITE_CODE, string_from_format("%d:%s", tcb->pid, buf));

	for(i = 0; i < 3 && cont; i++) {
		puts("Ciclo!");
		enviar_mensaje(msp_fd, message[i]);
		status[i] = recibir_mensaje(msp_fd);
		puts(status[i]->stream);
		if(status[i]->header.id == NOT_ENOUGH_MEMORY) {
			puts("No hay memoria?!");
			cont = 0;
			free(tcb);
			tcb = NULL;
		} else if(status[i]->header.id != OK_MEMORY) {
			errno = EBADMSG;
			puts("reservar_memoria");
			exit(EXIT_FAILURE);
		}
	}

	puts("Salio ciclo!");

	if(cont) {
		tcb->segmento_codigo = atoi((const char *) status[0]->stream);
		tcb->segmento_codigo_size = strlen(buf);
		tcb->puntero_instruccion = tcb->segmento_codigo;

		tcb->base_stack = atoi((const char *) status[1]->stream);
		tcb->cursor_stack = tcb->base_stack;
	}

	for(i = 0; i < 3; i++) {
		destroy_message(message[i]);
		destroy_message(status[i]);
	}

	return tcb;
}


t_hilo *ult_tcb(t_hilo *tcb)
{
	t_hilo *new = malloc(sizeof(*new));
	if(tcb == NULL) {
		/* New process. */
		new->pid = get_unique_id();
		new->tid = new->pid;
		new->kernel_mode = false;

	} else {
		/* New thread for existing process. */
		new->pid = tcb->pid;
		new->tid = get_unique_id();
		new->kernel_mode = false;
	}

	return new;
}

