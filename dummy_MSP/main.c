#include <utiles/utiles.h>
#include <commons/string.h>

FILE *beso;

void interpret_message(int sockfd, t_msg *recibido)
{
	t_msg *msg;
	char dato_cpu[4]; 
	switch(recibido->header.id) {
		case OC_REQUEST:
			printf("Recibi del CPU ID: %d Tamanio: %d\n\n",recibido->header.id,recibido->header.length);
			fread(dato_cpu,4,1,beso);
			printf("%s",dato_cpu);
			msg = string_message(NEXT_OC, dato_cpu, 0);	
			enviar_mensaje(sockfd, msg);
			destroy_message(msg);
			break;
		case ARG_REQUEST:
			printf("Recibi del CPU ID: %d Tamanio: %d\n\n",recibido->header.id,recibido->header.length);
			uint32_t arg_size;
			memcpy(&arg_size,recibido->stream + 8,recibido->header.length - 8);
			printf("arg_size: %d",arg_size);
			fread(dato_cpu,arg_size,1,beso);
			printf("%s",dato_cpu);
			msg = string_message(NEXT_ARG, dato_cpu, 0); 
			enviar_mensaje(sockfd, msg);
			destroy_message(msg);
			break;
		case RESERVE_SEGMENT:
			msg = string_message(OK_RESERVE, "0", 0);		
			enviar_mensaje(sockfd, msg);
			destroy_message(msg);
			break;
		case WRITE_MEMORY:
			msg = string_message(OK_WRITE, "0", 0);		
			enviar_mensaje(sockfd, msg);
			destroy_message(msg);
			break;
		default:
			return;
	}
}


int main(int argc, char **argv)
{
	fd_set master, read_fds;

	beso = fopen("A.bc","r");

	/* Create the socket and set it up to accept connections. */
	int listener = server_socket(2233);
	if(listener < 0) {
		perror("error: %d");
		exit(EXIT_FAILURE);
	}

	/* Initialize the set of active sockets. */
	FD_ZERO (&master);
	FD_SET (listener, &master);

	int fdmax = listener;

	while (1) {

		/* Block until input arrives on one or more active sockets. */
		memcpy(&read_fds, &master, sizeof(read_fds));

		if (select (fdmax + 1, &read_fds, NULL, NULL, NULL) < 0) {
			perror ("select");
			exit(EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		int i;

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET (i, &read_fds)) {
				if (i == listener) {

					/* Connection request on original socket. */
					int newfd = accept_connection(listener);
					if (newfd < 0) {
						perror ("accept");
						exit(EXIT_FAILURE);
					}
					puts("Conexion.");
					FD_SET (newfd, &master);
					fdmax = newfd > fdmax ? newfd : fdmax;
				}
				else {

					/* Data arriving on an already-connected socket. */
					t_msg *recibido = recibir_mensaje(i);
					if(recibido == NULL) {

						/* Socket closed connection. */
						puts("Desconexion.");
						close (i);
						FD_CLR (i, &master);
					} else {

						/* Socket received message. */
						putmsg(recibido);

						interpret_message(i, recibido);

						destroy_message(recibido);
					}	
				}
			}
		}
	}
}

