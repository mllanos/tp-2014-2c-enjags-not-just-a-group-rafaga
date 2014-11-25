#include <stdio.h>
#include <utiles/utiles.h>
main() {

	char * id;
	int fd = client_socket("127.0.0.1",2233);

	while(getchar() != EOF) {

		t_msg *msg = argv_message(DESTROY_SEGMENT, 2, 74, 1048576);
		
		enviar_mensaje(fd,msg);

		puts("Mensaje enviado");

		msg = recibir_mensaje(fd);

		printf("%s",id = id_string(msg->header.id));

		free(msg);
	
	}

}
