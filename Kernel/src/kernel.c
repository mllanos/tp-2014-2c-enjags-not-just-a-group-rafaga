#include "kernel.h"

int main(int argc, char **argv) {
	initialize(argv[1]);
	sleep(500);
	return 0;
}

void initialize(char *config_path) {
	pthread_t loader_th;
	pthread_t planificador_th;
	t_config *config = config_create(config_path);
	//inicializar_panel(KERNEL, PANEL_PATH);
	pthread_create(&loader_th, NULL, loader, (void *) config);
	pthread_create(&planificador_th, NULL, planificador, (void *) config);

}


