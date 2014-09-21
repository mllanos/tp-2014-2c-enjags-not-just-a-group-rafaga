#include <utiles/utiles.h>
#include <commons/string.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main (int argc, char **argv) {
	int sockfd = client_socket("127.0.0.1", 1122);
	while(getchar() != 'q') {
		t_msg *enviado = new_message(CONSOLE_CODE, "Enviando mensaje de prueba.");
		puts(enviado->stream);
		enviar_mensaje(sockfd, enviado);
		destroy_message(enviado);
	}
	close(sockfd);
	return 0;
}

