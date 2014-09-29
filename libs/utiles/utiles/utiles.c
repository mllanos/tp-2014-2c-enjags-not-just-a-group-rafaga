#include "utiles.h"

int server_socket(uint16_t port)
{
	int sockfd, optval = 1;
	struct sockaddr_in servername;

	/* Create the socket. */
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("socket");
		return -1;
	}

	/* Set socket options. */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		perror("setsockopt");
		return -2;
	}

	/* Fill ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = htonl(INADDR_ANY);
	servername.sin_port = htons(port);

 	/* Give the socket a name. */
	if(bind(sockfd, (struct sockaddr *) &servername, sizeof(servername)) < 0) {
		perror("bind");
		return -3;
	}

	/* Listen to incoming connections. */
	if (listen(sockfd, 1) < 0) {
		perror("listen");
		return -4;
	}

	return sockfd;
}


int client_socket(char *ip, uint16_t port)
{
	int sockfd;
	struct sockaddr_in servername;

	/* Create the socket. */
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("socket");
		return -1;
	}

	/* Fill server ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = inet_addr(ip);
	servername.sin_port = htons(port);
	memset(&(servername.sin_zero), 0, 8);

	/* Connect to the server. */
	if(connect(sockfd, (struct sockaddr *) &servername, sizeof (servername)) < 0) {
		perror("connect");
		return -2;
	}

	return sockfd;
}


int accept_connection(int sockfd)
{
	struct sockaddr_in clientname;
	size_t size = sizeof(clientname);

	int new_fd = accept(sockfd, (struct sockaddr *) &clientname, (socklen_t *) &size);
	if (new_fd < 0) {
		perror("accept");
		return -1;
	}	

	return new_fd;
}


t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...)
{
	va_list arguments;
	va_start(arguments, count);

	uint32_t *val = malloc(sizeof(*val) * count);

	int i;
	for(i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	t_msg *new = malloc(sizeof(*new));
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = strlen(message);
	new->stream = string_duplicate(message);

	va_end(arguments);

	return new;
}


t_msg *modify_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...)
{
	va_list arguments;
	va_start(arguments, new_count);

	uint32_t *val = malloc(sizeof(*val) * new_count);

	int i;
	for(i = 0; i < new_count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	char *buffer = malloc(sizeof(*buffer) * old_msg->header.length);

	memcpy(buffer, old_msg->stream, old_msg->header.length);

	t_msg *new = malloc(sizeof(*new));
	new->header.id = new_id;
	new->header.argc = new_count;
	new->argv = val;
	new->header.length = old_msg->header.length;
	new->stream = buffer;

	va_end(arguments);

	destroy_message(old_msg);

	return new;
}


t_msg *beso_message(t_msg_id id, char *beso_path, uint16_t count, ...)
{
	va_list arguments;
	va_start(arguments, count);

	uint32_t *val = malloc(sizeof(*val) * count);

	int i;
	for(i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	FILE *f = fopen(beso_path, "rb");
	if(f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(sizeof(*buffer) * (fsize + 1));

	fread(buffer, fsize, 1, f);

	buffer[fsize] = '\0';

	t_msg *new = malloc(sizeof(*new));
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = fsize;
	new->stream = buffer;

	va_end(arguments);

	fclose(f);

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
	t_msg *msg = malloc(sizeof(*msg));
	msg->argv = NULL;
	msg->stream = NULL;


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
	msg->argv = realloc(msg->argv, sizeof(uint32_t) * msg->header.argc);

	if(recv(sockfd, msg->argv, sizeof(uint32_t) * msg->header.argc, MSG_WAITALL) < 0) {
		perror("recv");
		exit(EXIT_FAILURE);
	}

	msg->stream = realloc(msg->stream, sizeof(char) * msg->header.length);

	if (recv(sockfd, msg->stream, sizeof(char) * msg->header.length, MSG_WAITALL) < 0) {
		perror("recv");
		exit(EXIT_FAILURE);
	}
	
	msg->stream[msg->header.length] = '\0';

	return msg;
}


t_msg *_recibir_mensaje(int sockfd)
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

	if (recv(sockfd, msg->stream, msg->header.length, MSG_WAITALL) < 0) {
		perror("recv");
		exit(EXIT_FAILURE);
	}
	
	msg->stream[msg->header.length] = '\0';

	return msg;
}


void enviar_mensaje(int sockfd, t_msg *msg)
{
	int total = 0;
	int pending = msg->header.length + sizeof(t_header) + sizeof(uint32_t) * msg->header.argc;
	char *buffer = malloc(pending);

 	/* Fill buffer with the struct's data. */
	memcpy(buffer, &(msg->header), sizeof(t_header));

	int i;
	for(i = 0; i < msg->header.argc; i++)
		memcpy(buffer + sizeof(t_header) + sizeof(uint32_t) * i, msg->argv + i, sizeof(uint32_t));

	memcpy(buffer + sizeof(t_header) + sizeof(uint32_t) * msg->header.argc, msg->stream, msg->header.length);

 	/* Send message(s). */
	while(total < pending) {
		int sent = send(sockfd, buffer, msg->header.length + sizeof(msg->header) + sizeof(uint32_t) * msg->header.argc, MSG_NOSIGNAL);
		if(sent < 0) {
			perror("send");
			exit(EXIT_FAILURE);
		}
		total += sent;
		pending -= sent;
	}

	free(buffer);
}



void _enviar_mensaje(int sockfd, t_msg *msg)
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
			exit(EXIT_FAILURE);
		}
		total += sent;
		pending -= sent;
	}

	free(buffer);
}


void destroy_message(t_msg *msg)
{
	free(msg->stream);
	free(msg->argv);
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


char *read_file(char *path)
{
	FILE *f = fopen(path, "rb");
	if(f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(sizeof(*buffer)*(fsize+1));
	if(buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, fsize, 1, f);

	buffer[fsize] = '\0';

	return buffer;
}

/*
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
*/


void putmsg(t_msg *msg)
{
	int i;
	puts("\n**************************************************");
	printf("Contenidos del mensaje:\n");
	printf("- ID: %d\n", msg->header.id);

	for(i = 0; i < msg->header.argc; i++) {;
		printf("- Argumento %d: %d\n", i + 1, msg->argv[i]);
	}

	printf("- Cuerpo: ");

	for(i = 0; i < msg->header.length; i++)
		putchar(*(msg->stream + i));
	puts("\n**************************************************\n");
}


