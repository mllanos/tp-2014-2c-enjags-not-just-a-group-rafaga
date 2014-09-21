#include <utiles/utiles.h>
#include <commons/string.h>

void interpret_message(int sockfd, t_msg *recibido)
{
	t_msg *msg;
	switch(recibido->header.id) {
		case RESERVE_CODE:
		case RESERVE_STACK:
		case WRITE_CODE:
			msg = new_message(OK_MEMORY, "0");		
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

	/* Create the socket and set it up to accept connections. */
	int listener = server_socket(2233);

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
						puts(recibido->stream);

						interpret_message(i, recibido);

						destroy_message(recibido);
					}	
				}
			}
		}
	}
}

