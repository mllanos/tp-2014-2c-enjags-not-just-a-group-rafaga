#include "kernel.h"

int main(int argc, char **argv)
{

	pthread_t loader_th;
	//pthread_t planificador_th;
	initialize(argv[1]);
	pthread_create(&loader_th, NULL, loader, NULL);
	boot_kernel();
	//pthread_create(&planificador_th, NULL, planificador, NULL);
	getchar(); /* TODO borrar. */
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
	msp_connect();
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


void msp_connect(void)
{
	char *ip = config_get_string_value(config, "IP_MSP");
	uint16_t port = config_get_int_value(config, "PUERTO_MSP");
	msp_fd = client_socket(ip, port);
}


t_hilo *klt_tcb(void)
{
	t_hilo *new = malloc(sizeof(*new));
	new->pid = 0;
	new->tid = new->pid;
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

