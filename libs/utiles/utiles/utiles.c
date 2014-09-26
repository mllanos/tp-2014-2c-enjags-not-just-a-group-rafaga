#include "utiles.h"

int server_socket(uint16_t port)
{
	return _server_socket(port, "socket", "setsockopt", "bind", "listen");
}


int client_socket(char *ip, uint16_t port)
{
	return _client_socket(ip, port, "socket", "connect");
}


int accept_connection(int sockfd)
{
	return _accept_connection(sockfd, "accept");
}


t_msg *new_message(t_msg_id id, char *message)
{
	t_msg *new = malloc(sizeof(*new));
	new->header.id = id;
	new->header.length = strlen(message);
	char *stream = string_duplicate(message);
	new->stream = stream;
	return new;
}

t_msg *crear_mensaje(t_msg_id id, char *message,uint32_t size)
{
	t_msg *new = malloc(sizeof(*new));
	new->header.id = id;
	new->header.length = size;
	new->stream = message;
	return new;
}


t_msg *recibir_mensaje(int sockfd)
{
	return _recibir_mensaje(sockfd, "recv");
}


void enviar_mensaje(int sockfd, t_msg *msg)
{
	_enviar_mensaje(sockfd, msg, "send");
}


void destroy_message(t_msg *msg)
{
	free(msg->stream);
	free(msg);
}


int max(int a, int b)
{
	return a < b ? b : a;
}


int randomize(int limit)
{
	return rand() % (limit + 1);
}


long seedgen(void)
{
	long s, seed, pid;
	time_t seconds;
	pid = getpid();
    s = time (&seconds); /* get CPU seconds since 01/01/1970 */
	seed = abs(((s*181)*((pid-83)*359))%104729);
	return seed;
}


int msleep(useconds_t usecs)
{
	return usleep(usecs*1000);
}


char *serializar_tcb(t_hilo *tcb,uint16_t quantum) {

	int i = 2,j;
	char *stream = malloc(sizeof *tcb + sizeof quantum);

	memcpy(stream,&quantum,sizeof quantum);
	memcpy(stream + 2,&tcb->pid,REG_SIZE);
	memcpy(stream + (i+= REG_SIZE),&tcb->tid,REG_SIZE);
	memcpy(stream + (i+= REG_SIZE),&tcb->kernel_mode,sizeof tcb->kernel_mode);
	memcpy(stream + (i+= sizeof tcb->kernel_mode),&tcb->segmento_codigo,REG_SIZE);
	memcpy(stream + (i+= REG_SIZE),&tcb->segmento_codigo_size,REG_SIZE);
	memcpy(stream + (i+= REG_SIZE),&tcb->puntero_instruccion,REG_SIZE);
	memcpy(stream + (i+= REG_SIZE),&tcb->base_stack,REG_SIZE);
	memcpy(stream + (i+= REG_SIZE),&tcb->cursor_stack,REG_SIZE);
	for(j = 0;j < 5;++j)
		memcpy(stream + (i+= REG_SIZE),&tcb->registros[j],REG_SIZE);
	memcpy(stream + (i+= REG_SIZE),&tcb->cola,sizeof tcb->cola);

	return stream;

}


void deserializar_tcb(t_hilo *tcb, char *stream) {

	int i = 0,j;

	memcpy(&tcb->pid,stream + i,REG_SIZE);
	memcpy(&tcb->tid,stream + (i+= REG_SIZE),REG_SIZE);
	memcpy(&tcb->kernel_mode,stream + (i+= REG_SIZE),sizeof tcb->kernel_mode);
	memcpy(&tcb->segmento_codigo,stream + (i+= sizeof tcb->kernel_mode),REG_SIZE);
	memcpy(&tcb->segmento_codigo_size,stream + (i+= REG_SIZE),REG_SIZE);
	memcpy(&tcb->puntero_instruccion,stream + (i+= REG_SIZE),REG_SIZE);
	memcpy(&tcb->base_stack,stream + (i+= REG_SIZE),REG_SIZE);
	memcpy(&tcb->cursor_stack,stream + (i+= REG_SIZE),REG_SIZE);
	for(j = 0;j < 5;++j)
		memcpy(&tcb->registros[j],stream + (i+= REG_SIZE),REG_SIZE);
	memcpy(&tcb->cola,stream + (i+= REG_SIZE),sizeof tcb->cola);

}


int _server_socket(uint16_t port, char *e_socket, char *e_setsockopt, char *e_bind, char *e_listen)
{
	int sockfd, optval = 1;
	struct sockaddr_in servername;

	/* Create the socket. */
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror(e_socket);
		exit(EXIT_FAILURE);
	}

	/* Set socket options. */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		perror(e_setsockopt);
		exit(EXIT_FAILURE);
	}

	/* Fill ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = htonl(INADDR_ANY);
	servername.sin_port = htons(port);

 	/* Give the socket a name. */
	if(bind(sockfd, (struct sockaddr *) &servername, sizeof(servername)) < 0) {
		perror(e_bind);
		exit(EXIT_FAILURE);
	}

	/* Listen to incoming connections. */
	if (listen(sockfd, 1) < 0) {
		perror (e_listen);
		exit (EXIT_FAILURE);
	}

	return sockfd;
}


int _client_socket(char* ip, uint16_t port, char *e_socket, char *e_connect)
{
	int sockfd;
	struct sockaddr_in servername;

	/* Create the socket. */
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror(e_socket);
		exit(EXIT_FAILURE);
	}

	/* Fill server ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = inet_addr(ip);
	servername.sin_port = htons(port);
	memset(&(servername.sin_zero), 0, 8);

	/* Connect to the server. */
	if(connect(sockfd, (struct sockaddr *) &servername, sizeof (servername)) < 0) {
		perror(e_connect);
		exit(EXIT_FAILURE);
	}

	return sockfd;
}


int socket_cliente(char* ip, char * port){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(ip, port, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	return serverSocket;

}

int _accept_connection(int sockfd, char *e_accept)
{
	struct sockaddr_in clientname;
	size_t size = sizeof(clientname);

	int new_fd = accept (sockfd, (struct sockaddr *) &clientname, (socklen_t *) &size);
	if (new_fd < 0) {
		perror (e_accept);
		exit(EXIT_FAILURE);
	}	

	return new_fd;
}


t_msg *_recibir_mensaje(int sockfd, char *e_recv)
{
	t_msg *msg = malloc(sizeof(*msg));
	msg->stream = string_new();

 	/* Get message info. */
	int status = recv(sockfd, &(msg->header), sizeof(t_header), MSG_WAITALL);
	if (status < 0) {
 		/* An error has ocurred. */
		perror(e_recv);
		exit(EXIT_FAILURE);
	} else if (status == 0) {
 		/* Remote connection has been closed. */
		free(msg->stream);
		free(msg);
		return NULL;
	}

 	/* Get message data. */
	msg->stream = realloc(msg->stream, sizeof(char) * msg->header.length);

	if (recv(sockfd, msg->stream, msg->header.length, MSG_WAITALL) < 0) {
		perror(e_recv);
		exit(EXIT_FAILURE);
	}
	
	msg->stream[msg->header.length] = '\0';

	return msg;
}


void _enviar_mensaje(int sockfd, t_msg *msg, char *e_send)
{
	int total = 0;
	int pending = msg->header.length + sizeof(t_header);
	int sent;
	char *buffer = malloc(sizeof(t_header) + msg->header.length);

 	/* Fill buffer with the struct's data. */
	memcpy(buffer, &(msg->header), sizeof(t_header));
	memcpy(buffer + sizeof(t_header), msg->stream, msg->header.length);

 	/* Send message(s). */
	while(total < pending) {
		sent = send(sockfd, buffer, msg->header.length + sizeof(msg->header), MSG_NOSIGNAL);
		if(sent < 0) {
			perror(e_send);
			exit(EXIT_FAILURE);
		}
		total += sent;
		pending -= sent;
	}

	free(buffer);
}
