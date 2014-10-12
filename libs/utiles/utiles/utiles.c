#include "utiles.h"

int server_socket(uint16_t port)
{
	int sock_fd, optval = 1;
	struct sockaddr_in servername;

	/* Create the socket. */
	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0) {
		perror("socket");
		return -1;
	}

	/* Set socket options. */
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1) {
		perror("setsockopt");
		return -2;
	}

	/* Fill ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = htonl(INADDR_ANY);
	servername.sin_port = htons(port);

 	/* Give the socket a name. */
	if(bind(sock_fd, (struct sockaddr *) &servername, sizeof servername) < 0) {
		perror("bind");
		return -3;
	}

	/* Listen to incoming connections. */
	if (listen(sock_fd, 1) < 0) {
		perror("listen");
		return -4;
	}

	return sock_fd;
}


int client_socket(char *ip, uint16_t port)
{
	int sock_fd;
	struct sockaddr_in servername;

	/* Create the socket. */
	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0) {
		perror("socket");
		return -1;
	}

	/* Fill server ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = inet_addr(ip);
	servername.sin_port = htons(port);
	memset(&(servername.sin_zero), 0, 8);

	/* Connect to the server. */
	if(connect(sock_fd, (struct sockaddr *) &servername, sizeof servername) < 0) {
		perror("connect");
		return -2;
	}

	return sock_fd;
}


int accept_connection(int sock_fd)
{
	struct sockaddr_in clientname;
	size_t size = sizeof clientname;

	int new_fd = accept(sock_fd, (struct sockaddr *) &clientname, (socklen_t *) &size);
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

	uint32_t *val = malloc(count * sizeof *val);

	int i;
	for(i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	t_msg *new = malloc(sizeof *new);
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

	uint16_t old_count = old_msg->header.argc;

	uint32_t *val = malloc((new_count + old_count) * sizeof *val);
	uint32_t *val_old = val + new_count;

	int i;
	for(i = 0; i < new_count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	val_old = memcpy(val + new_count, old_msg->argv, old_count * sizeof(uint32_t));

	char *buffer = malloc(old_msg->header.length);

	memcpy(buffer, old_msg->stream, old_msg->header.length);

	t_msg *new = malloc(sizeof *new);
	new->header.id = new_id == NO_NEW_ID ? old_msg->header.id : new_id;
	new->header.argc = new_count + old_count;
	new->argv = val;
	new->header.length = old_msg->header.length;
	new->stream = buffer;

	va_end(arguments);

	destroy_message(old_msg);

	return new;
}


t_msg *remake_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...)
{
	va_list arguments;
	va_start(arguments, new_count);

	uint32_t *val = malloc(new_count * sizeof *val);

	int i;
	for(i = 0; i < new_count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	char *buffer = malloc(old_msg->header.length);

	memcpy(buffer, old_msg->stream, old_msg->header.length);

	t_msg *new = malloc(sizeof *new);
	new->header.id = new_id == NO_NEW_ID ? old_msg->header.id : new_id;
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

	uint32_t *val = malloc(count * sizeof *val);

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

	char *buffer = malloc(fsize + 1);

	fread(buffer, fsize, 1, f);

	t_msg *new = malloc(sizeof *new);
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = fsize;
	new->stream = buffer;

	va_end(arguments);

	fclose(f);

	return new;
}


t_msg *tcb_message(t_msg_id id, t_hilo *tcb, uint16_t count, ...)
{
	va_list arguments;
	va_start(arguments, count);

	uint32_t *val = malloc(count * sizeof *val);

	int i;
	for(i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	size_t size = sizeof *tcb;
	char *buffer = malloc(size + 1);

	memcpy(buffer, tcb, size);

	t_msg *new = malloc(sizeof *new);
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = size;
	new->stream = buffer;

	va_end(arguments);

	return new;
}


t_msg *crear_mensaje(t_msg_id id, char *message,uint32_t size)
{
	t_msg *new = malloc(sizeof *new);
	new->header.id = id;
	new->header.length = size;
	new->stream = message;
	return new;
}


t_msg *recibir_mensaje(int sock_fd)
{
	t_msg *msg = malloc(sizeof *msg);
	msg->argv = NULL;
	msg->stream = NULL;


 	/* Get message info. */
	int status = recv(sock_fd, &(msg->header), sizeof(t_header), MSG_WAITALL);
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
	msg->argv = realloc(msg->argv, msg->header.argc * sizeof(uint32_t));

	if(msg->header.argc > 0 && recv(sock_fd, msg->argv, msg->header.argc * sizeof(uint32_t), MSG_WAITALL) < 0) {
		perror("recv");
		exit(EXIT_FAILURE);
	}

	msg->stream = realloc(msg->stream, msg->header.length);

	if (msg->header.length > 0 && recv(sock_fd, msg->stream, msg->header.length, MSG_WAITALL) < 0) {
		perror("recv");
		exit(EXIT_FAILURE);
	}
	
	return msg;
}


void enviar_mensaje(int sock_fd, t_msg *msg)
{
	int total = 0;
	int pending = msg->header.length + sizeof(t_header) + msg->header.argc * sizeof(uint32_t);
	char *buffer = malloc(pending);

 	/* Fill buffer with the struct's data. */
	memcpy(buffer, &(msg->header), sizeof(t_header));

	int i;
	for(i = 0; i < msg->header.argc; i++)
		memcpy(buffer + sizeof(t_header) + i * sizeof(uint32_t), msg->argv + i, sizeof(uint32_t));

	memcpy(buffer + sizeof(t_header) + msg->header.argc * sizeof(uint32_t), msg->stream, msg->header.length);

 	/* Send message(s). */
	while(total < pending) {
		int sent = send(sock_fd, buffer, msg->header.length + sizeof msg->header + msg->header.argc * sizeof(uint32_t), MSG_NOSIGNAL);
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

void seedgen(void)
{
	long s, seed, pid;
	time_t seconds;
	pid = getpid();
    	s = time (&seconds); /* get CPU seconds since 01/01/1970 */
	seed = abs(((s*181)*((pid-83)*359))%104729);
	srand(seed);
}

char *serializar_tcb(t_hilo *tcb,uint16_t quantum)
{

	int i = 2,j;
	char* stream;

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

t_hilo *retrieve_tcb(t_msg *msg)
{

	t_hilo *new = malloc(sizeof *new);
	memcpy(new, msg->stream, sizeof *new);
	return new;
}

void deserializar_tcb(t_hilo *tcb, char *stream)
{

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

	char *buffer = malloc(fsize + 1);
	if(buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, fsize, 1, f);

	buffer[fsize] = '\0';

	return buffer;
}

void clean_stdin_buffer(void)
{
	scanf ("%*[^\n]");
}


void putmsg(t_msg *msg)
{
	int i;
	puts("\n**************************************************");
	printf("CONTENIDOS DEL MENSAJE:\n");
	printf("- ID: %s\n", id_string(msg->header.id));

	for(i = 0; i < msg->header.argc; i++) {;
		printf("- Argumento %d: %d\n", i + 1, msg->argv[i]);
	}

	printf("Tamaño: %d\n", msg->header.length);
	printf("- Cuerpo: ");

	for(i = 0; i < msg->header.length; i++)
		putchar(*(msg->stream + i));
	puts("\n**************************************************\n");
}

char *id_string(t_msg_id id)
{
	switch(id) {
		case NO_NEW_ID:
			return "NO_NEW_ID";
		case INIT_CONSOLE:
			return "INIT_CONSOLE";
		case KILL_CONSOLE:
			return "KILL_CONSOLE";
		case NUMERIC_INPUT:
			return "NUMERIC_INPUT";
		case REPLY_INPUT:
			return "REPLY_INPUT";
		case STRING_INPUT:
			return "STRING_INPUT";
		case STRING_OUTPUT:
			return "STRING_OUTPUT";
		case CREATE_SEGMENT:
			return "CREATE_SEGMENT";
		case OK_CREATE:
			return "OK_CREATE";
		case FULL_MEMORY:
			return "FULL_MEMORY";
		case INVALID_SEG_SIZE:
			return "INVALID_SEG_SIZE";
		case MAX_SEG_NUM_REACHED:
			return "MAX_SEG_NUM_REACHED";
		case WRITE_MEMORY:
			return "WRITE_MEMORY";
		case OK_WRITE:
			return "OK_WRITE";
		case SEGMENTATION_FAULT:
			return "SEGMENTATION_FAULT";
		case INVALID_DIR:
			return "INVALID_DIR";
		case DESTROY_SEGMENT:
			return "DESTROY_SEGMENT";
		case REQUEST_MEMORY:
			return "REQUEST_MEMORY";
		case OK_REQUEST:
			return "OK_REQUEST";
		case NEXT_THREAD:
			return "NEXT_THREAD";
		case CPU_TCB:
			return "CPU_TCB";
		case CPU_CONNECT:
			return "CPU_CONNECT";
		case CPU_INTERRUPT:
			return "CPU_INTERRUPT";
		case CPU_THREAD:
			return "CPU_THREAD";
		case CPU_JOIN:
			return "CPU_JOIN";
		case CPU_BLOCK:
			return "CPU_BLOCK";
		case CPU_WAKE:
			return "CPU_WAKE";
		default:
			return string_from_format("%d, <AGREGAR A LA LISTA>", id);
	}
}
