#include "kernel.h"

int main(int argc, char **argv) {
	initialize(argv[1]);
	getchar(); /* temp */
	return 0;
}

void initialize(char *config_path) {
	pthread_t loader_th;
	pthread_t planificador_th;
	config = config_create(config_path);
	new_queue = queue_create();
	ready_queue = queue_create();
	exec_queue = queue_create();
	block_queue = queue_create();
	exit_queue = queue_create();
	inicializar_panel(KERNEL, PANEL_PATH);
	pthread_create(&loader_th, NULL, loader, NULL);
	pthread_create(&planificador_th, NULL, planificador, NULL);
}


