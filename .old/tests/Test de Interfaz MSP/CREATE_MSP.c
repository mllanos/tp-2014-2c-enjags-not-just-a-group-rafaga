#include <stdio.h>
#include <utiles/utiles.h>
main() {

	char * id;
	int fd = client_socket("127.0.0.1",2233);

	while(getchar() != EOF) {

		t_msg *msg = argv_message(CREATE_SEGMENT, 2, 74, 10240);
		
		enviar_mensaje(fd,msg);

		puts("Mensaje enviado");

		msg = recibir_mensaje(fd);

		printf("%s\n%u",id = id_string(msg->header.id),msg->argv[0]);

		free(msg);
	
	}

}
