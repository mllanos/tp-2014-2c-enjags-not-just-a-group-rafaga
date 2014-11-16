#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utiles/utiles.h>

int main(void) {
	t_msg *msg[100];
	int sock_fd = client_socket("127.0.0.1", 1122);
	if(sock_fd < 0) {
		puts("No se pudo conectar con el Kernel.");
		return 1;
	}
	int i;
	for(i = 0; i < 100; i++)
		msg[i] = beso_message(KILL_CONSOLE, "../file.bin", 0);

	for(i = 0; i < 100; i++) {
		int j = enviar_mensaje(sock_fd, msg[i]);
		printf("Enviado mensaje nro. %d, length %d.\n", i + 1, j);
		destroy_message(msg[i]);
	}

	return 0;
}
