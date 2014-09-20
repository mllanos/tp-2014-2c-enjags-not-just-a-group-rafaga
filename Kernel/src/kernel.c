#include "kernel.h"

int main(int argc, char **argv)
{
	pthread_t loader_th;
	pthread_t planificador_th;
	initialize(argv[1]);
	msp_connect();
	pthread_create(&loader_th, NULL, loader, NULL);
	pthread_create(&planificador_th, NULL, planificador, NULL);
	getchar(); /* temp */
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
	inicializar_panel(KERNEL, PANEL_PATH);
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
}

void msp_connect(void)
{
	char *ip = config_get_string_value(config, "IP_MSP");
	uint16_t port = config_get_int_value(config, "PUERTO_MSP");
	//int msp_fd = client_socket(ip, port);
}

void kernel_tcb(void)
{
	t_hilo *new = malloc(sizeof(*new));
		new->pid = 0;
		new->tid = new->pid;
		new->kernel_mode = true;

	queue_push(block_queue, new);
}