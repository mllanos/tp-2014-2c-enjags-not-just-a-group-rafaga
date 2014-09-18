#include "loader.h"

int read_from_client(int filedes)
{
	char buffer[MAXMSG];
	int nbytes;

	nbytes = read(filedes, buffer, MAXMSG);
	if (nbytes < 0) {
    	/* Read error. */
		perror ("read");
		exit (EXIT_FAILURE);
	}
	else if (nbytes == 0) {
    	/* End-of-file. */
		return -1;
	}
	else {
    	/* Data read. */
		fprintf(stderr, "Server: got message: `%s'\n", buffer);
		return 0;
	}
}

void *loader(void *arg)
{
	t_config *config = (t_config *) arg;
	fd_set active_fd_set, read_fd_set;
	int i;
	struct sockaddr_in clientname;
	size_t size = sizeof (clientname);

	/* Create the socket and set it up to accept connections. */
	int listener = server_socket(config_get_int_value(config, "PUERTO"), &clientname);

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (listener, &active_fd_set);
	//int max_fd = listener;

	while (1) {

		/* Block until input arrives on one or more active sockets. */
		memcpy(&read_fd_set, &active_fd_set, sizeof(read_fd_set));

		if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			perror ("select");
			exit (EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET (i, &read_fd_set)) {
				if (i == listener) {

					/* Connection request on original socket. */
					int new_console = accept (listener, (struct sockaddr *) &clientname, (socklen_t *) &size);
					if (new_console < 0) {
						perror ("accept");
						exit (EXIT_FAILURE);
					}
					fprintf (stderr, "Server: connect from host %s, port %hd.\n", inet_ntoa (clientname.sin_addr), ntohs (clientname.sin_port));
					FD_SET (new_console, &active_fd_set);
					t_msg *msg = new_message(0, "Hello new connection. - Kernel");
					enviar_mensaje(new_console, msg);
					destroy_message(msg);
				}
				else {

					/* Data arriving on an already-connected socket. */

					t_msg *recibido = recibir_mensaje(i);
					puts(recibido->stream);

					close (i);
					FD_CLR (i, &active_fd_set);

					destroy_message(recibido);

					/*
					if (read_from_client (i) < 0) {
						fprintf (stderr, "Server: disconnect from host %s, port %hd.\n", inet_ntoa (clientname.sin_addr), ntohs (clientname.sin_port));
						close (i);
						FD_CLR (i, &active_fd_set);
					}
					*/
				}
			}
		}
	}
}
