#include <utiles/utiles.h>
#include <commons/process.h>
#include <stdio.h>
#include <pthread.h>

void *threaded_sender(void *arg) {
	int sock_fd = *((int *) arg);
	t_msg *bin_msg = beso_message(KILL_CONSOLE, "../file.bin", 0);
	int j = enviar_mensaje(sock_fd, bin_msg);
	printf("<<%d>>: Enviados %d caracteres a Kernel.\n", process_get_thread_id(), j);
	destroy_message(bin_msg);
}

int main(void) {
	pthread_t thread[100];
	int sock_fd = client_socket("127.0.0.1", 1122);
	if (sock_fd < 0) {
		puts("No se pudo conectar con Kernel.");
		return 1;
	}
	int i;
	for(i = 0; i < 100; i++) {
		pthread_create(&thread[i], NULL, threaded_sender, &sock_fd);
		pthread_detach(thread[i]);
	}

	sleep(4);
	
	return 0;
}


