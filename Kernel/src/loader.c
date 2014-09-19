#include "loader.h"
#include "kernel.h"

void *loader(void *arg)
{
	fd_set active_fd_set, read_fd_set;
	int i;
	struct sockaddr_in clientname;
	size_t size = sizeof (clientname);

	/* Create the socket and set it up to accept connections. */
	int listener = server_socket(config_get_int_value(config, "PUERTO"), &clientname);

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (listener, &active_fd_set);

	while (1) {

		/* Block until input arrives on one or more active sockets. */
		memcpy(&read_fd_set, &active_fd_set, sizeof(read_fd_set));

		if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			perror ("select");
			exit(EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET (i, &read_fd_set)) {
				if (i == listener) {

					/* Connection request on original socket. */
					int new_console = accept (listener, (struct sockaddr *) &clientname, (socklen_t *) &size);
					if (new_console < 0) {
						perror ("accept");
						exit(EXIT_FAILURE);
					}
					fprintf (stderr, "Server: connect from host %s, port %hd.\n", inet_ntoa (clientname.sin_addr), ntohs (clientname.sin_port));
					FD_SET (new_console, &active_fd_set);
					//t_msg *msg = new_message(0, "Hello new connection. - Kernel");
					//enviar_mensaje(new_console, msg);
					//destroy_message(msg);
				}
				else {

					/* Data arriving on an already-connected socket. */
					t_msg *recibido = recibir_mensaje(i);
					puts(recibido->stream);

					t_msg *e_msg;
					t_hilo *tcb;

					switch(recibido->header.id) {
						case CONSOLE_CODE:
							tcb = reservar_memoria(recibido->stream);
							
							if(tcb == NULL) {
								/* Couldn't allocate memory. */
								e_msg = new_message(NOT_ENOUGH_MEMORY, "No se pudo reservar memoria en la MSP.");
								enviar_mensaje(i, e_msg);
								destroy_message(e_msg);
								close (i);
								FD_CLR (i, &active_fd_set);
							} else {
								/* Initialize registers and push process to the new queue. */
								int i;
								for(i = 0; i < 5; i++)
									tcb->registros[i] = 0;
								t_process *new_proc = crear_proceso(i, tcb);
								queue_push(new_queue, new_proc);
							}
							break;
						case CONSOLE_OUT:
							fprintf (stderr, "Server: disconnect from host %s, port %hd.\n", inet_ntoa (clientname.sin_addr), ntohs (clientname.sin_port));
							close (i);
							FD_CLR (i, &active_fd_set);
							break;
						default:
							puts("Unknown ID");
							exit(EXIT_FAILURE);
					}

					destroy_message(recibido);
				}
			}
		}
	}
}

uint32_t get_unique_id(void)
{
	static int x = 0;
	return x++;
}

t_hilo *reservar_memoria(char *beso_data)
{
	t_hilo *tcb = new_tcb(NULL);
	char *ip = config_get_string_value(config, "IP_MSP");
	uint16_t port = config_get_int_value(config, "PUERTO_MSP");
	int msp_fd = client_socket(ip, port);

	/* Sending the process' PID to MSP. */
	t_msg *process = new_message(NEW_PROCESS, string_itoa(tcb->pid));
	enviar_mensaje(msp_fd, process);

	/* Sending the process' program. */
	t_msg *code = new_message(RESERVE_CODE, beso_data);
	enviar_mensaje(msp_fd, code);

	t_msg *status_code = recibir_mensaje(msp_fd);

	switch(status_code->header.id == NOT_ENOUGH_MEMORY) {
		case NOT_ENOUGH_MEMORY:
			return NULL;
		case OK_MEMORY:
			tcb->segmento_codigo = atoi((const char *) status_code->stream);
			tcb->segmento_codigo_size = strlen(beso_data);
			tcb->puntero_instruccion = tcb->segmento_codigo;
			break;
		default:
			puts("Unknown ID.");
			exit(EXIT_FAILURE);
	}

	/* Sending the process' stack size. */
	t_msg *stack = new_message(RESERVE_STACK, config_get_string_value(config, "TAMANIO_STACK"));
	enviar_mensaje(msp_fd, stack);

	t_msg *status_stack = recibir_mensaje(msp_fd);
	switch(status_stack->header.id == NOT_ENOUGH_MEMORY) {
		case NOT_ENOUGH_MEMORY:
			return NULL;
		case OK_MEMORY:
			tcb->base_stack = atoi((const char *) status_stack->stream);
			tcb->cursor_stack = tcb->base_stack;
			break;
		default:
			puts("Unknown ID.");
			exit(EXIT_FAILURE);
	}

	destroy_message(process);
	destroy_message(code);
	destroy_message(status_code);
	destroy_message(stack);
	destroy_message(status_stack);
	close(msp_fd);

	return tcb;
}

t_hilo *new_tcb(t_hilo *tcb)
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

t_process *crear_proceso(int sockfd, t_hilo *tcb)
{
	t_process *proc = malloc(sizeof(*proc));
	proc->sockfd = sockfd;
	proc->tcb = tcb;
	return proc;
}

void eliminar_proceso(t_process *proc)
{
	free(proc->tcb);
	free(proc);
}