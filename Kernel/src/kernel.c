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

	msp_connect();

	pthread_create(&loader_th, NULL, loader, NULL);
	pthread_create(&planificador_th, NULL, planificador, NULL);
}


void boot_kernel(void)
{
	int i;
	t_hilo *k_tcb;

	k_tcb = reservar_memoria(klt_tcb(), get_syscalls());
	if(k_tcb == NULL) {
		/* Couldn't allocate memory. */
		errno = ENOMEM;
		perror("boot_kernel");
		exit(EXIT_FAILURE);
	}

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
						puts(recibido->stream);

						interpret_message(i, recibido);

						destroy_message(recibido);
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
			queue_push(loader_queue, string_from_format("%d|%s", sockfd, recibido->stream));
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


void msp_connect(void)
{
	char *ip = config_get_string_value(config, "IP_MSP");
	uint16_t port = config_get_int_value(config, "PUERTO_MSP");
	msp_fd = client_socket(ip, port);
}




uint32_t get_unique_id(void)
{
	static int x = 0;
	return ++x;
}


t_hilo *reservar_memoria(t_hilo *tcb, char *buf)
{
	int i = 0, cont = 1;
	t_msg *message[5]; /* 0-2 mensajes, 3-4 status */
	t_msg **status = message + 3;

	message[0] = new_message(RESERVE_CODE, string_from_format("%d:%d", tcb->pid, strlen(buf)));
	message[1] = new_message(RESERVE_STACK, string_from_format("%d:%d", tcb->pid, get_stack_size()));
	message[2] = new_message(WRITE_CODE, string_from_format("%d:%s", tcb->pid, buf));

	do {
		enviar_mensaje(msp_fd, message[i]);
		puts(message[i]->stream);
		
		if(i < 2) {
			status[i] = recibir_mensaje(msp_fd);
			puts(status[i]->stream);
		
			if(status[i]->header.id == NOT_ENOUGH_MEMORY) {
				cont = 0;
				free(tcb);
				tcb = NULL;
			} else if(status[i]->header.id != OK_MEMORY) {
				errno = EBADMSG;
				perror("reservar_memoria");
				exit(EXIT_FAILURE);
			}
		}

	} while (i < 2 && cont && ++i);

	if(cont) {
		tcb->segmento_codigo = atoi((const char *) status[0]->stream);
		tcb->segmento_codigo_size = strlen(buf);
		tcb->puntero_instruccion = tcb->segmento_codigo;

		tcb->base_stack = atoi((const char *) status[1]->stream);
		tcb->cursor_stack = tcb->base_stack;
	}

	for(i = 0; i < 5; i++) {
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
	char *location = config_get_string_value(config, "SYSCALLS");

	int fd = open(location, O_RDONLY);
	if(fd < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	off_t len = lseek(fd, 0, SEEK_END);
	if(len < 0) {
		perror("lseek");
		exit(EXIT_FAILURE);
	}

	char *contents = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if(contents == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	return contents;
}

