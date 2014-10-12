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
	process_list = list_create();
	console_list = list_create();
	cpu_list = list_create();
	resource_list = list_create();
	loader_queue = queue_create();
	planificador_queue = queue_create();
	syscall_queue = queue_create();
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
	t_hilo *_klt_tcb(void) {
		t_hilo *new = malloc(sizeof(*new));
		new->pid = 0;
		new->tid = 0;
		new->kernel_mode = true;
		return new;
	}

	t_hilo *k_tcb = reservar_memoria(_klt_tcb(), beso_message(INIT_CONSOLE, get_syscalls(), 0));
	if(k_tcb == NULL) {
		/* Couldn't allocate memory. */
		errno = ENOMEM;
		perror("boot_kernel");
		exit(EXIT_FAILURE);
	}

	int i;
	for(i = 0; i < 5; i++)
		k_tcb->registros[i] = 0;
	k_tcb->cola = BLOCK;

	list_add(process_list, k_tcb);
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
		for (i = 0; i <= fdmax; i++)
			if (FD_ISSET (i, &read_fds))
				if (i == listener) {
					/* Connection request on original socket. */
					int newfd = accept_connection(listener);
					if (newfd < 0) {
						perror ("accept");
						exit(EXIT_FAILURE);
					}
					FD_SET (newfd, &master);
					fdmax = newfd > fdmax ? newfd : fdmax;
				}
				else {
					/* Data arriving on an already-connected socket. */
					t_msg *recibido = recibir_mensaje(i);
					if(recibido == NULL) {
						/* Socket closed connection. */
						int status = remove_from_lists(i);
						close(i);
						FD_CLR(i, &master);

						if(status == -1) {
							/* Exit program. */
							close(listener);
							return;
						}
					} else {
						/* Socket received message. */
						putmsg(recibido);
						interpret_message(i, recibido);
					}	
				}
	}
}


void finalize(void)
{
	void _resource_destroyer(t_resource *res) {
		queue_destroy_and_destroy_elements(res->queue, (void *) free);
		free(res);
	}

	config_destroy(config);
	list_destroy_and_destroy_elements(process_list, (void *) free);
	list_destroy_and_destroy_elements(console_list, (void *) free);
	list_destroy_and_destroy_elements(cpu_list, (void *) free);
	list_destroy_and_destroy_elements(resource_list, (void *) _resource_destroyer);
	queue_destroy_and_destroy_elements(loader_queue, (void *) destroy_message);
	queue_destroy_and_destroy_elements(planificador_queue, (void *) destroy_message);
	queue_destroy_and_destroy_elements(syscall_queue, (void *) free);
	sem_destroy(&sem_loader);
	sem_destroy(&sem_planificador);
	pthread_mutex_destroy(&loader_mutex);
	pthread_mutex_destroy(&planificador_mutex);
	pthread_kill(loader_th, SIGTERM);
	pthread_kill(planificador_th, SIGTERM);
}


void interpret_message(int sock_fd, t_msg *recibido)
{
	switch(recibido->header.id) {
		/* Mensaje de Consola. */
		case INIT_CONSOLE: 										/* <BESO_STRING> */
			
			pthread_mutex_lock(&loader_mutex);
			queue_push(loader_queue, remake_message(NO_NEW_ID, recibido, 1, sock_fd));
			pthread_mutex_unlock(&loader_mutex);

			sem_post(&sem_loader);

			break;
		/* Mensaje de CPU. */
		case CPU_CONNECT:										/* <[STRING]> */
		case CPU_TCB:											/* <[STRING]> */
		case RETURN_TCB:										/* <TCB_STRING> */
		case CPU_INTERRUPT: 									/* <MEM_DIR, TCB_STRING> */
		case NUMERIC_INPUT: 									/* <PID, [STRING]> */
		case STRING_INPUT: 										/* <PID, [STRING]> */
		case STRING_OUTPUT: 									/* <PID, OUT_STRING> */
		case CPU_THREAD:										/* <TCB_STRING> */
		case CPU_JOIN:											/* <CALLER_TID, WAITER_TID, [STRING]> */
		case CPU_BLOCK:											/* <RESOURCE_ID, TCB_STRING> */
		case CPU_WAKE:											/* <RESOURCE_ID, [STRING]> */
			
			pthread_mutex_lock(&planificador_mutex);
			queue_push(planificador_queue, modify_message(NO_NEW_ID, recibido, 1, sock_fd));
			pthread_mutex_unlock(&planificador_mutex);

			sem_post(&sem_planificador);

			break;
		/* Mensaje para CPU de Consola. */
		case REPLY_INPUT:										/* <CPU_SOCK_FD, REPLY_STRING> */
			pthread_mutex_lock(&planificador_mutex);
			queue_push(planificador_queue, recibido);
			pthread_mutex_unlock(&planificador_mutex);

			sem_post(&sem_planificador);

			break;
		default: 												/* Nunca deberia pasar. */
			errno = EBADMSG;
			perror("interpret_message");
			exit(EXIT_FAILURE);
	}
}



t_hilo *reservar_memoria(t_hilo *tcb, t_msg *msg)
{
	int i = 0, cont = 1;
	t_msg *message[6];											/* 0-2 mensajes, 3-5 status. */
	t_msg **status = message + 3;

	message[0] = string_message(RESERVE_SEGMENT, "Reserva de segmento de codigo.", 2, tcb->pid, msg->header.length);
	message[1] = string_message(RESERVE_SEGMENT, "Reserva de segmento de stack.", 2, tcb->pid, get_stack_size());
	message[2] = remake_message(WRITE_MEMORY, msg, 1, tcb->pid);

	for(i = 0; i < 3 && cont; i++) {
		enviar_mensaje(msp_fd, message[i]);
		putmsg(message[i]);

		status[i] = recibir_mensaje(msp_fd);
		putmsg(status[i]);
		
		if(i < 2) {												/* Status de reserva de memoria. */
			if(status[i]->header.id == ENOMEM_RESERVE) {
				cont = 0;
				free(tcb);
				tcb = NULL;
			} else if(status[i]->header.id != OK_RESERVE) {
				errno = EBADMSG;
				perror("reservar_memoria");
				exit(EXIT_FAILURE);
			}
		} else {												/* Status de escritura de codigo. */
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
		tcb->segmento_codigo = status[0]->argv[0];
		tcb->segmento_codigo_size = msg->header.length;
		tcb->puntero_instruccion = tcb->segmento_codigo;
		tcb->base_stack = status[1]->argv[0];
		tcb->cursor_stack = tcb->base_stack;
	}

	for(i = 0; i < 6; i++)
		destroy_message(message[i]);

	return tcb;
}


int remove_from_lists(uint32_t sock_fd)
{
	bool _remove_by_sock_fd_cnsl(t_console *console) { 
		return console->sock_fd == sock_fd; 
	}

	t_console *out_console = list_remove_by_condition(console_list, (void *) _remove_by_sock_fd_cnsl);

	bool _remove_by_sock_fd_cpu(t_cpu *cpu) { 
		return cpu->sock_fd == sock_fd; 
	}

	t_cpu *out_cpu = list_remove_by_condition(cpu_list, (void *) _remove_by_sock_fd_cpu);

	if(out_console != NULL) { /* Es una consola. */
		desconexion_consola(out_console->console_id);

		/* Finalizar todos los procesos de la consola. */

		void _finalize_process_from_pid(t_hilo *tcb) {
			if(tcb->pid == out_console->pid && tcb->kernel_mode == false) tcb->cola = EXIT;
		}

		list_iterate(process_list, (void *) _finalize_process_from_pid);

		free(out_console);
	} else if(out_cpu != NULL) { /* Es una CPU. */
		desconexion_cpu(out_cpu->cpu_id);

		if(out_cpu->kernel_mode == false) { /* La CPU saliente tiene un hilo comun ejecutando. */

			/* Borrar los procesos del CPU y avisar a consola. */

			void _notify_from_pid(t_hilo *tcb) {
				if(tcb->pid == out_cpu->pid && tcb->pid == tcb->tid) { /* Hilo principal. */
					tcb->cola = EXIT;

					bool _has_console(t_console *cons) {
						return cons->pid == tcb->pid && tcb->kernel_mode == false;
					}

					t_console *out_cons = list_remove_by_condition(console_list, (void *) _has_console);

					t_msg *msg = string_message(KILL_CONSOLE, "Finalizando consola. Motivo: CPU saliente.", 0);
					enviar_mensaje(out_cons->sock_fd, msg);
					destroy_message(msg);
				} else if(tcb->pid == out_cpu->pid) { /* Hilo secundario. */
					tcb->cola = EXIT;
				}
			}

			list_iterate(process_list, (void *) _notify_from_pid);
		} else { /* La CPU saliente tiene el hilo kernel.*/

			/* Finalizar todos los procesos y salir del programa. */

			void _notify_all_consoles(t_console *cnsl) {
				t_msg *msg = string_message(KILL_CONSOLE, "Finalizando consola. Motivo: CPU saliente.", 0);
				enviar_mensaje(cnsl->sock_fd, msg);
				destroy_message(msg);
			}

			list_iterate(console_list, (void *) _notify_all_consoles);

			return -1;
		}

		free(out_cpu);
	}

	return 0;
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


uint32_t get_unique_id(t_unique_id id)
{
	static int tid = 0;
	static int console_id = 0;
	static int cpu_id = 0;

	int ret = -1;

	switch(id) {
		case THREAD_ID:
			ret = ++tid;
			break;
		case CONSOLE_ID:
			ret = ++console_id;
			break;
		case CPU_ID:
			ret = ++cpu_id;
			break;
		default:
			break;
	}

	return ret;
}