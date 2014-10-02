#include "planificador.h"

void *planificador(void *arg)
{
	while(1) {
		sem_wait(&sem_planificador);
		
		pthread_mutex_lock(&planificador_mutex);
		t_msg *recibido = queue_pop(planificador_queue);
		pthread_mutex_unlock(&planificador_mutex);

		switch(recibido->header.id) {
			case CPU_CONNECT:
				//cpu_add(sockfd);
				break;
			case CPU_PROCESS:
				//assign_process(sockfd);
				break;
			case CPU_DISCONNECT:
				//disconnect_cpu(sockfd);
				break;
			case CPU_INTERRUPT:
				//system_call(args[2]);
				break;
			case NUMERIC_INPUT:
			case STRING_INPUT:
				//standard_input(args[2]);
				break;
			case STRING_OUTPUT:
				//standard_output(args[2]);
				break;
			case CPU_THREAD:
				//create_thread(sockfd, args[2]);
				break;
			case CPU_JOIN:
				//join_thread(args[2]);
				break;
			case CPU_BLOCK:
				//block_thread(args[2]);
				break;
			case CPU_WAKE:
				//wake_thread(args[2]);
				break;
			default:
				errno = EBADMSG;
				perror("planificador");
				break;
		}
	}
}

void disconnect_cpu(int sockfd)
{
	console_notify(sockfd); /* Chequear si cpu tiene proceso. */
	//abort_processes();
}

void cpu_add(int sockfd)
{
	t_cpu *new = malloc(sizeof(*new));

	new->cpu_fd = sockfd;
	new->active_tcb = NULL;

	list_add(cpu_list, new);
}


void console_notify(int sockfd)
{
	t_cpu *cpu = cpu_remove(sockfd);
	if(cpu == NULL) {
		perror("notificar_consola");
		exit(EXIT_FAILURE);
	}

	if(cpu->active_tcb != NULL) {
		// TODO remove t_hilo
	}
}


t_cpu *cpu_remove(int sockfd)
{
	int i;
	for(i = 0; i < list_size(cpu_list); i++) {
		t_cpu *cpu = list_get(cpu_list, i);

		if(cpu->cpu_fd == sockfd) {
			return list_remove(cpu_list, i);
		}
	}

	return NULL;
}


void create_thread(int sockfd, char *tcb_stream)
{
	t_hilo *tcb = get_tcb(tcb_stream);
	t_hilo *new_tcb = malloc(sizeof(*new_tcb));

	new_tcb->pid = tcb->pid;
	new_tcb->tid = get_unique_id();
	new_tcb->kernel_mode = tcb->kernel_mode;

	t_msg *r_stack = string_message(RESERVE_SEGMENT, "Reservando segmento para nuevo thread.", 0, new_tcb->pid, get_stack_size());

	enviar_mensaje(msp_fd, r_stack);

	t_msg *status = recibir_mensaje(msp_fd);

	if(status->header.id == OK_RESERVE) {
		queue_push(ready_queue, new_tcb);
	} else if(status->header.id == ENOMEM_RESERVE) {
		free(new_tcb);
	} else {
		errno = EBADMSG;
		perror("create_thread");
		exit(EXIT_FAILURE);	
	}

	free(tcb);
}

t_hilo *get_tcb(char *tcb_stream)
{
	int i, j = 0;

	t_hilo *tcb = malloc(sizeof(*tcb));

	char **args = string_split(tcb_stream, ":");

	tcb->pid = atoi(args[0]);
	tcb->tid = atoi(args[1]);
	tcb->kernel_mode = atoi(args[2]);
	tcb->segmento_codigo = atoi(args[3]);
	tcb->segmento_codigo_size = atoi(args[4]);
	tcb->puntero_instruccion = atoi(args[5]);
	tcb->base_stack = atoi(args[6]);
	tcb->cursor_stack = atoi(args[7]);
	for(i = 8; i < 13; i++)
		tcb->registros[i] = atoi(args[i]);

	while(args[j] != NULL)
		free(args[j++]);
	
	return tcb;
}