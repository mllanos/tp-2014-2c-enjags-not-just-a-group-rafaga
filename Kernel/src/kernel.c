#include "kernel.h"

int main(int argc, char **argv)
{
	initialize(argv[1]);
	boot_kernel();
	receive_messages_epoll();
	finalize();
	return EXIT_SUCCESS;
}


void initialize(char *config_path)
{
	config = config_create(config_path);
	logger = log_create(LOG_PATH, "Kernel", true, LOG_LEVEL_TRACE);
	process_list = list_create();
	console_list = list_create();
	cpu_list = list_create();
	resource_dict = dictionary_create();
	join_dict = dictionary_create();
	father_child_dict = dictionary_create();
	loader_queue = queue_create();
	planificador_queue = queue_create();
	syscall_queue = queue_create();
	request_queue = queue_create();
	pthread_mutex_init(&loader_mutex, NULL);
	pthread_mutex_init(&planificador_mutex, NULL);
	pthread_mutex_init(&console_list_mutex, NULL);
	pthread_mutex_init(&cpu_list_mutex, NULL);
	pthread_mutex_init(&process_list_mutex, NULL);
	pthread_mutex_init(&unique_id_mutex[CONSOLE_ID], NULL);
	pthread_mutex_init(&unique_id_mutex[CPU_ID], NULL);
	sem_init(&sem_loader, 0, 0);
	sem_init(&sem_planificador, 0, 0);
	inicializar_panel(KERNEL, PANEL_PATH);
	pthread_create(&loader_th, NULL, loader, NULL);
	pthread_create(&planificador_th, NULL, planificador, NULL);

	msp_fd = client_socket(get_ip_msp(), get_puerto_msp());
	if (msp_fd < 0) {
		log_error(logger, "No se pudo conectar con la MSP.");
		exit(EXIT_FAILURE);
	}

	log_trace(logger, "Inicializando el proceso Kernel.");
}


void boot_kernel(void)
{
	log_trace(logger, "Reservando memoria para el el hilo de Kernel.");

	klt_tcb = malloc(sizeof *klt_tcb);
	klt_tcb->pid = 0;
	klt_tcb->tid = 0;
	klt_tcb->kernel_mode = true;
	klt_tcb->cola = BLOCK;
	memset(klt_tcb->registros, 0, sizeof klt_tcb->registros);

	klt_tcb = reservar_memoria(klt_tcb, beso_message(INIT_CONSOLE, get_syscalls(), 0));
	if (klt_tcb == NULL) {
		/* Couldn't allocate memory. */
		log_error(logger, "No se pudo reservar memoria para el hilo de Kernel.");
		errno = ENOMEM;
		perror("boot_kernel");
		exit(EXIT_FAILURE);
	}

	log_trace(logger, "Bloqueando el hilo de Kernel.");

	list_add(process_list, klt_tcb);
}


void receive_messages_epoll(void)
{
	struct epoll_event event;
	struct epoll_event *events;

	memset(&event, 0, sizeof(event));

	int sfd = server_socket(get_puerto());
	if(sfd < 0) {
		log_error(logger, "No se pudo crear socket escucha.");
		perror("receive_messages_epoll");
		exit(EXIT_FAILURE);
	}


	int efd = epoll_create1(0);
	if (efd == -1) {
		perror ("epoll_create");
		exit(EXIT_FAILURE);
	}

	event.data.fd = sfd;
	event.events = EPOLLIN;
	int s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if (s == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}

	/* Buffer where events are returned. */
	events = calloc(MAXEVENTS, sizeof event);

	/* The event loop. */
	while(1) {
		int n, i;

		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
			/* An error has occured on this fd, or the socket is not ready for reading. */
				perror("epoll error");
				close(events[i].data.fd);
			} else if (sfd == events[i].data.fd) {
				/* We have a notification on the listening socket, which means one incoming connection. */
				int infd = accept_connection(sfd);

				/* Make the incoming socket non-blocking and add it to the list of fds to monitor. */
				s = make_socket_non_blocking(infd);
				if (s == -1) {
					exit(EXIT_FAILURE);
				}

				event.data.fd = infd;
				event.events = EPOLLIN;

				s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
				if (s == -1) {
					perror("epoll_ctl");
					exit(EXIT_FAILURE);
				}
			} else {
				/* We have data on the fd waiting to be read. */

				t_msg *msg = recibir_mensaje(events[i].data.fd);
				if (msg == NULL) {
					int status = remove_from_lists(events[i].data.fd);

					/* Closing the descriptor will make epoll remove it from the set of descriptors which are monitored. */
					close(events[i].data.fd);

					if (status == -1) {
						/* Exit program. */
						free(events);
						close(sfd);
						return;
					}
				} else
					interpret_message(events[i].data.fd, msg);
			}
		}
	}
}


void receive_messages_select(void)
{
	fd_set master, read_fds;

	/* Create the socket and set it up to accept connections. */
	int listener = server_socket(get_puerto());
	if(listener < 0) {
		log_error(logger, "No se pudo crear socket escucha.");
		perror("receive_messages_select");
		exit(EXIT_FAILURE);
	}

	/* Initialize the set of active sockets. */
	FD_ZERO(&master);
	FD_SET(listener, &master);

	int fdmax = listener;

	while (1) {
		/* Block until input arrives on one or more active sockets. */
		memcpy(&read_fds, &master, sizeof(read_fds));

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		}
		/* Service all the sockets with input pending. */
		int i;
		for (i = 0; i <= fdmax; i++)
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					/* Connection request on original socket. */
					int newfd = accept_connection(listener);
					if (newfd < 0) {
						perror ("accept");
						exit(EXIT_FAILURE);
					}
					FD_SET(newfd, &master);
					fdmax = newfd > fdmax ? newfd : fdmax;
				}
				else {
					/* Data arriving on an already-connected socket. */
					t_msg *recibido = recibir_mensaje(i);
					if (recibido == NULL) {
						/* Socket closed connection. */
						int status = remove_from_lists(i);
						close(i);
						FD_CLR(i, &master);

						if (status == -1) {
							/* Exit program. */
							close(listener);
							return;
						}
					} else {
						/* Socket received message. */
						interpret_message(i, recibido);
					}
				}
			}
	}
}


void finalize(void)
{
	pthread_kill(loader_th, SIGTERM);
	pthread_kill(planificador_th, SIGTERM);

	destroy_segments_on_exit_or_condition(true);

	sem_destroy(&sem_loader);
	sem_destroy(&sem_planificador);
	pthread_mutex_destroy(&loader_mutex);
	pthread_mutex_destroy(&planificador_mutex);
	pthread_mutex_destroy(&console_list_mutex);
	pthread_mutex_destroy(&cpu_list_mutex);
	pthread_mutex_destroy(&process_list_mutex);
	pthread_mutex_destroy(&unique_id_mutex[CONSOLE_ID]);
	pthread_mutex_destroy(&unique_id_mutex[CPU_ID]);
	config_destroy(config);
	log_destroy(logger);
	list_destroy_and_destroy_elements(process_list, (void *) free);
	list_destroy_and_destroy_elements(console_list, (void *) free);
	list_destroy_and_destroy_elements(cpu_list, (void *) free);
	dictionary_destroy_and_destroy_elements(resource_dict, (void *) free);
	dictionary_destroy_and_destroy_elements(join_dict, (void *) free);
	dictionary_destroy(father_child_dict);
	queue_destroy_and_destroy_elements(loader_queue, (void *) destroy_message);
	queue_destroy_and_destroy_elements(planificador_queue, (void *) destroy_message);
	queue_destroy_and_destroy_elements(syscall_queue, (void *) free);
	queue_destroy_and_destroy_elements(request_queue, (void *) free);
}


void interpret_message(int sock_fd, t_msg *recibido)
{
	/* Tipos de mensaje: <[stream]; [argv, [argv, ]*]> */

	print_msg(recibido);

	switch (recibido->header.id) {
		/* Mensaje de conexion de Consola. */
		case INIT_CONSOLE:  									/* <BESO_STRING;> */
			pthread_mutex_lock(&loader_mutex);
			queue_push(loader_queue, modify_message(NO_NEW_ID, recibido, 1, sock_fd));
			pthread_mutex_unlock(&loader_mutex);
			sem_post(&sem_loader);
			break;
		/* Mensajes de CPU que necesitan el cpu_sock_fd. */
		case CPU_CONNECT:										/* <;> */
		case CPU_TCB:											/* <;> */
		case CPU_ABORT:											/* <TCB_STRING;> */
		case CPU_CREA:											/* <TCB_STRING;> */
		case RETURN_TCB:										/* <TCB_STRING;> */
		case FINISHED_THREAD:									/* <TCB_STRING;> */
		case NUMERIC_INPUT: 									/* <; PID> */
		case STRING_INPUT: 										/* <; PID, SIZE> */
			pthread_mutex_lock(&planificador_mutex);
			queue_push(planificador_queue, modify_message(NO_NEW_ID, recibido, 1, sock_fd));
			pthread_mutex_unlock(&planificador_mutex);
			sem_post(&sem_planificador);
			break;
		/* Mensajes de CPU que no necesitan el cpu_sock_fd. */
		case CPU_INTERRUPT: 									/* <TCB_STRING; MEM_DIR> */
		case CPU_JOIN:											/* <; CALLER_TID, WAITER_TID, PROCESS_PID> */
		case CPU_BLOCK:											/* <TCB_STRING; RESOURCE_ID> */
		case CPU_WAKE:											/* <RESOURCE_ID> */
		case NUMERIC_OUTPUT:									/* <; PID, NUMERIC> */
		case STRING_OUTPUT: 									/* <OUT_STRING; PID> */
		/* Mensajes de Consola para delegar a CPU. */
		case REPLY_STRING_INPUT:								/* <REPLY_STRING; CPU_SOCK_FD> */
		case REPLY_NUMERIC_INPUT:								/* <; CPU_SOCK_FD, REPLY_NUMERIC> */
			pthread_mutex_lock(&planificador_mutex);
			queue_push(planificador_queue, recibido);
			pthread_mutex_unlock(&planificador_mutex);
			sem_post(&sem_planificador);
			break;
		default: 												/* Nunca deberia pasar. */
			errno = EBADMSG;
			log_error(logger, "ID desconocido en interpret_message.");
			perror("interpret_message");
			exit(EXIT_FAILURE);
	}
}


t_hilo *reservar_memoria(t_hilo *tcb, t_msg *msg)
{
	int i, cont = 1;
	t_msg *message[6];
	t_msg **status = message + 3;

	message[0] = argv_message(CREATE_SEGMENT, 2, tcb->pid, msg->header.length);
	message[1] = argv_message(CREATE_SEGMENT, 2, tcb->pid, get_stack_size());

	for (i = 0; i < 2 && cont; i++) {
		enviar_mensaje(msp_fd, message[i]);

		status[i] = recibir_mensaje(msp_fd);

		if (MSP_RESERVE_FAILURE(status[i]->header.id)) {
			free(tcb);
			tcb = NULL;
			cont = 0;
		} else if (!MSP_RESERVE_SUCCESS(status[i]->header.id)) {
			log_error(logger, "ID desconocido en reservar_memoria.");
			errno = EBADMSG;
			perror("reservar_memoria");
			exit(EXIT_FAILURE);
		}
	}

	if(cont) {
		message[2] = remake_message(WRITE_MEMORY, msg, 2, tcb->pid, status[0]->argv[0]);

		enviar_mensaje(msp_fd, message[2]);

		status[2] = recibir_mensaje(msp_fd);

		if (MSP_WRITE_FAILURE(status[2]->header.id)) {
			free(tcb);
			tcb = NULL;
		} else if (!MSP_WRITE_SUCCESS(status[2]->header.id)) {
			log_error(logger, "ID desconocido en reservar_memoria.");
			errno = EBADMSG;
			perror("reservar_memoria");
			exit(EXIT_FAILURE);
		}

		if(tcb != NULL) {
			tcb->segmento_codigo = status[0]->argv[0];
			tcb->segmento_codigo_size = message[2]->header.length;
			tcb->puntero_instruccion = tcb->segmento_codigo;
			tcb->base_stack = status[1]->argv[0];
			tcb->cursor_stack = tcb->base_stack;
		}

		destroy_message(message[2]);
		destroy_message(status[2]);
	}

	for (i = 0; i < 2; i++) {
		destroy_message(message[i]);
		destroy_message(status[i]);
	}

	return tcb;
}


int remove_from_lists(uint32_t sock_fd)
{
	int result = 0;
	
	t_console *out_console = remove_console_by_sock_fd(sock_fd);
	t_cpu *out_cpu = remove_cpu_by_sock_fd(sock_fd);

	if (out_console != NULL) {
		/* Es una consola, finalizar todos sus procesos. Verificar que ninguno sea el hilo Kernel. */
		//desconexion_consola(out_console->console_id);

		log_trace(logger, "Desconexion Consola %u.", out_console->pid);

		finalize_process_by_pid(out_console->pid);

		free(out_console);
	} else if (out_cpu != NULL) {
		/* Es una CPU. Verificar si tiene hilos ejecutando. */
		//desconexion_cpu(out_cpu->cpu_id);

		log_trace(logger, "Desconexion CPU %u.", out_cpu->cpu_id);

		if (out_cpu->disponible == false) {
			/* La CPU saliente tiene hilos ejecutando. */

			if (out_cpu->kernel_mode == false) {
				/* Es un ULT. Avisar a la Consola para que se desconecte. */
				t_console *out_cons = find_console_by_pid(out_cpu->pid);

				t_msg *msg = string_message(KILL_CONSOLE, "Finalizando consola. Motivo: CPU saliente.", 0);
				enviar_mensaje(out_cons->sock_fd, msg);
				destroy_message(msg);

			} else 
				/* Es el KLT. Liberar recursos y salir del programa. */
				result = -1;

			free(out_cpu);
		}
	} else {
		/* No es ni CPU ni Consola. */
		errno = EBADF;
		perror("remove_from_lists");
		exit(EXIT_FAILURE);
	}

	return result;
}


uint32_t get_unique_id(t_unique_id id)
{
	static uint32_t unique_id[2] = {0, 0};
	uint32_t ret;

	pthread_mutex_lock(&unique_id_mutex[id]);
	ret = ++unique_id[id];
	pthread_mutex_unlock(&unique_id_mutex[id]);

	return ret;
}


static int make_socket_non_blocking(int sfd)
{
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1) {
		perror("fcntl");
		return -1;
	}

	return 0;
}
