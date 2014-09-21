#include "utiles.h"

int server_socket(uint16_t port, struct sockaddr_in *name)
{
	int sock, optval = 1;

	/* Create the socket. */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Set socket options. */
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	/* Fill ip / port info. */
	name->sin_family = AF_INET;
	name->sin_addr.s_addr = htonl(INADDR_ANY);
	name->sin_port = htons(port);

 	/* Give the socket a name. */
	if(bind(sock, (struct sockaddr *) name, sizeof (*name)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	/* Listen to incoming connections. */
	if (listen(sock, 1) < 0) {
		perror ("listen");
		exit (EXIT_FAILURE);
	}

	return sock;
}

int client_socket(char* ip, uint16_t port)
{
	int sock;
	struct sockaddr_in servername;

	/* Create the socket. */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Fill server ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = inet_addr(ip);
	servername.sin_port = htons(port);
	memset(&(servername.sin_zero), 0, 8);

	/* Connect to the server. */
	if(connect(sock, (struct sockaddr *) &servername, sizeof (servername)) < 0) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	return sock;
}

 t_msg *new_message(t_msg_id id, char *message)
 {
 	t_msg *new = malloc(sizeof(*new));
 	new->header.id = id;
 	new->header.length = strlen(message);
 	new->stream = string_duplicate(message);
 	return new;
 }

 t_msg *recibir_mensaje(int sockfd)
 {
 	t_msg *msg = malloc(sizeof(*msg));
 	msg->stream = string_new();

 	/* Get message info. */
 	int status = recv(sockfd, &(msg->header), sizeof(t_header), MSG_WAITALL);
 	if (status < 0) {
 		/* An error has ocurred. */
 		perror("recv");
 		exit(EXIT_FAILURE);
 	} else if (status == 0) {
 		/* Remote connection has been closed. */
 		free(msg->stream);
 		free(msg);
 		return NULL;
 	}

 	/* Get message data. */
 	msg->stream = realloc(msg->stream, sizeof(char) * msg->header.length);
 	if (msg->header.length > 0) {
 		if (recv(sockfd, msg->stream, msg->header.length, MSG_WAITALL) < 0) {
 			perror("recv");
 			exit(EXIT_FAILURE);
 		}
 	}

 	return msg;
 }

 void enviar_mensaje(int sockfd, t_msg *msg)
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
 			perror("send");
 		}
 		total += sent;
 		pending -= sent;
 	}

 	free(buffer);
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

 char *serializador_tcb(t_hilo *tcb,uint16_t quantum) {

	 int i = 2,j;
	 char *stream = malloc(sizeof *tcb + sizeof quantum);

	 memcpy(stream,&quantum,sizeof quantum);
	 memcpy(stream + 2,&tcb->pid,REG_SIZE);
	 memcpy(stream + (i+= REG_SIZE),&tcb->tid,REG_SIZE);
	 memcpy(stream + (i+= REG_SIZE),&tcb->kernel_mode,REG_SIZE);
	 memcpy(stream + (i+= REG_SIZE),&tcb->segmento_codigo,REG_SIZE);
	 memcpy(stream + (i+= REG_SIZE),&tcb->segmento_codigo_size,REG_SIZE);
	 memcpy(stream + (i+= REG_SIZE),&tcb->puntero_instruccion,REG_SIZE);
	 memcpy(stream + (i+= REG_SIZE),&tcb->base_stack,REG_SIZE);
	 memcpy(stream + (i+= REG_SIZE),&tcb->cursor_stack,REG_SIZE);
	 for(j = 0;j < 5;++j)
		 memcpy(stream + (i+= REG_SIZE),&tcb->registros[j],REG_SIZE);
	 memcpy(stream + (i+= REG_SIZE),&tcb->cola,sizeof tcb->cola);

	 return stream;

 }

 void deserializador_tcb(t_hilo *tcb, char *stream) {

	 int i = 0,j;

	 memcpy(&tcb->pid,stream + i,REG_SIZE);
	 memcpy(&tcb->tid,stream + (i+= REG_SIZE),REG_SIZE);
	 memcpy(&tcb->kernel_mode,stream + (i+= REG_SIZE),REG_SIZE);
	 memcpy(&tcb->segmento_codigo,stream + (i+= REG_SIZE),REG_SIZE);
	 memcpy(&tcb->segmento_codigo_size,stream + (i+= REG_SIZE),REG_SIZE);
	 memcpy(&tcb->puntero_instruccion,stream + (i+= REG_SIZE),REG_SIZE);
	 memcpy(&tcb->base_stack,stream + (i+= REG_SIZE),REG_SIZE);
	 memcpy(&tcb->cursor_stack,stream + (i+= REG_SIZE),REG_SIZE);
	 for(j = 0;j < 5;++j)
		 memcpy(&tcb->registros[j],stream + (i+= REG_SIZE),REG_SIZE);
	 memcpy(&tcb->cola,stream + (i+= REG_SIZE),sizeof tcb->cola);

 }
