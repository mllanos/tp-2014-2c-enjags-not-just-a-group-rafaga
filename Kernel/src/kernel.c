#include "kernel.h"

int main(int argc, char **argv)
{
	initialize(argv[1]);
	boot_kernel();
	receive_messages();
	finalize();
	return 0;
}


void initialize(char *config_path)
{
	config = config_create(config_path);
	sockfd_dict = dictionary_create();
	new_queue = queue_create();
	ready_queue = queue_create();
	exec_queue = queue_create();
	block_queue = queue_create();
	exit_queue = queue_create();
	loader_queue = queue_create();
	planificador_queue = queue_create();
	cpu_list = list_create();
	pthread_mutex_init(&new_mutex, NULL);
	pthread_mutex_init(&loader_mutex, NULL);
	pthread_mutex_init(&planificador_mutex, NULL);
	sem_init(&sem_loader, 0, 0);
	sem_init(&sem_planificador, 0, 0);
	inicializar_panel(KERNEL, PANEL_PATH);
	msp_fd = client_socket(get_ip_msp(), get_puerto_msp());
	pthread_create(&loader_th, NULL, loader, NULL);
	pthread_create(&planificador_th, NULL, planificador, NULL);
}


void boot_kernel(void)
{
	t_hilo *k_tcb = reservar_memoria(klt_tcb(), beso_message(INIT_CONSOLE, get_syscalls(), 0));
	if(k_tcb == NULL) {
		/* Couldn't allocate memory. */
		errno = ENOMEM;
		perror("boot_kernel");
		exit(EXIT_FAILURE);
	}

	int i;
	for(i = 0; i < 5; i++)
		k_tcb->registros[i] = 0;

	queue_push(block_queue, k_tcb);
}


void receive_messages(void)
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

						putmsg(recibido);

						interpret_message(i, recibido);
					}	
				}
			}
		}
	}
}


void finalize(void)
{
	config_destroy(config);
	dictionary_destroy(sockfd_dict);
	queue_destroy(new_queue);
	queue_destroy(ready_queue);
	queue_destroy(exec_queue);
	queue_destroy(block_queue);
	queue_destroy(exit_queue);
	queue_destroy(loader_queue);
	queue_destroy(planificador_queue);
	list_destroy(cpu_list);
	sem_destroy(&sem_loader);
	sem_destroy(&sem_planificador);
	pthread_mutex_destroy(&loader_mutex);
	pthread_mutex_destroy(&planificador_mutex);
	pthread_kill(loader_th, SIGTERM);
	pthread_kill(planificador_th, SIGTERM);
}


void interpret_message(int sockfd, t_msg *recibido)
{
	switch(recibido->header.id) {
		case INIT_CONSOLE: /* Delegar a loader. */
			sem_post(&sem_loader);

			pthread_mutex_lock(&loader_mutex);
			queue_push(loader_queue, modify_message(INIT_CONSOLE, recibido, 0, 1, sockfd));
			pthread_mutex_unlock(&loader_mutex);

			break;
		case CPU_CONNECT: /* Delegar a planificador. */
		case CPU_PROCESS:
		case CPU_DISCONNECT:
		case CPU_INTERRUPT:
		case CPU_INPUT:
		case CPU_OUTPUT:
		case CPU_THREAD:
		case CPU_JOIN:
		case CPU_BLOCK:
		case CPU_WAKE:
			sem_post(&sem_planificador);

			pthread_mutex_lock(&planificador_mutex);
			queue_push(planificador_queue, string_from_format("%d|%d|%s", sockfd, recibido->header.id, recibido->stream));
			pthread_mutex_unlock(&planificador_mutex);

			break;
		default: /* Nunca deberia pasar. */
			errno = EBADMSG;
			perror("interpret_message");
			exit(EXIT_FAILURE);
	}
}



t_hilo *reservar_memoria(t_hilo *tcb, t_msg *msg)
{
	int i = 0, cont = 1;
	t_msg *message[6]; /* 0-2 mensajes, 3-5 status. */
	t_msg **status = message + 3;

	message[0] = string_message(RESERVE_SEGMENT, "Reserva de segmento de codigo.", 2, tcb->pid, msg->header.length);
	message[1] = string_message(RESERVE_SEGMENT, "Reserva de segmento de stack.", 2, tcb->pid, get_stack_size());
	message[2] = modify_message(WRITE_MEMORY, msg, 1, 1, tcb->pid);

	for(i = 0; i < 3 && cont; i++) {
		enviar_mensaje(msp_fd, message[i]);
		putmsg(message[i]);

		status[i] = recibir_mensaje(msp_fd);
		putmsg(status[i]);
		
		if(i < 2) { /* Status de reserva de memoria. */
			if(status[i]->header.id == ENOMEM_RESERVE) {
				cont = 0;
				free(tcb);
				tcb = NULL;
			} else if(status[i]->header.id != OK_RESERVE) {
				errno = EBADMSG;
				perror("reservar_memoria");
				exit(EXIT_FAILURE);
			}
		} else { /* Status de escritura de codigo. */
			if(status[i]->header.id == SEGFAULT_WRITE) {
				cont = 0;
				free(tcb);
				tcb = NULL;
			} else if(status[i]->header.id != OK_WRITE) {
				errno = EBADMSG;
				perror("reservar_memoria");
				exit(EXIT_FAILURE);
			}
		}

	} 

	if(cont) {
		tcb->segmento_codigo = atoi((const char *) status[0]->stream);
		tcb->segmento_codigo_size = msg->header.length;
		tcb->puntero_instruccion = tcb->segmento_codigo;

		tcb->base_stack = atoi((const char *) status[1]->stream);
		tcb->cursor_stack = tcb->base_stack;
	}

	for(i = 0; i < 6; i++) {
		destroy_message(message[i]);
	}

	return tcb;
}


t_hilo *klt_tcb(void)
{
	t_hilo *new = malloc(sizeof(*new));
	new->pid = 0;
	new->tid = 0;
	new->kernel_mode = true;

	return new;
}



int get_puerto(void)
{
	return config_get_int_value(config, "PUERTO");
}


char *get_ip_msp(void)
{
	return config_get_string_value(config, "IP_MSP");
}


int get_puerto_msp(void)
{
	return config_get_int_value(config, "PUERTO_MSP");
}


int get_quantum(void)
{
	return config_get_int_value(config, "QUANTUM");
}


int get_stack_size(void)
{
	return config_get_int_value(config, "TAMANIO_STACK");
}


char *get_syscalls(void)
{
	return config_get_string_value(config, "SYSCALLS");
}


uint32_t get_unique_id(void)
{
	static int x = 0;
	return ++x;
}