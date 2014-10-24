#include <stdio.h>
#include <utiles/utiles.h>
main() {

	char * id;
	int fd = client_socket("127.0.0.1",2233);

	while(getchar() != EOF) {

		t_msg *msg = string_message(WRITE_MEMORY, "Holaaaa Migueeeel;/*-1!", 3, 74, 127, 30);
		
		enviar_mensaje(fd,msg);

		puts("Mensaje enviado");

		msg = recibir_mensaje(fd);

		printf("%s",id = id_string(msg->header.id));

		free(msg);
	
	}

}
