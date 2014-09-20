#include <utiles/utiles.h>
#include <commons/string.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main (int argc, char **argv) {
	int sockfd = client_socket("127.0.0.1", 1122);
	t_msg *enviado = new_message(CONSOLE_CODE, "Enviando mensaje de prueba. - Console");
	puts(enviado->stream);
	printf("Length: %d\n", enviado->header.length);
	enviar_mensaje(sockfd, enviado);
	//t_msg *recibido = recibir_mensaje(sockfd);
	//puts(recibido->stream);
	sleep(5);
	t_msg *out = new_message(CONSOLE_OUT, "Blah.");
	enviar_mensaje(sockfd, out);
	close(sockfd);
	destroy_message(enviado);
	destroy_message(out);
	return 0;
}

