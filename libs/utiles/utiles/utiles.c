#include "utiles.h"

/*
 * Prepara un socket servidor.
 */
 int serverSocket(uint16_t port, struct sockaddr_in *name)
 {
 	int sock, yes = 1;

	/* Create the socket. */
 	sock = socket(PF_INET, SOCK_STREAM, 0);
 	if(sock < 0) {
 		perror("socket");
 		exit(EXIT_FAILURE);
 	}

	/* Set socket options. */
 	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
 		perror("setsockopt");
 		exit(1);
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

/*
 * Prepara un socket cliente.
 */
 int clientSocket(char* ip, uint16_t port)
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


/*
 * Enviar Mensaje.
 *
 */
 void enviarMensaje(int sockfd, char* info)
 {
 	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
 	mensaje->id = '_';
 	mensaje->info = info;
 	t_stream* stream = serializador(mensaje);
 	if((send(sockfd, stream->data, stream->length, 0)) < 0) {
 		perror("Error en el send");
 		exit(EXIT_FAILURE);
 	}
 }

/*
 * Recibir Mensaje.
 *
 */
 t_mensaje* recibirMensaje(int sockfd)
 {
 	char buffer[256];
 	int nbytes = recv(sockfd, buffer, sizeof(buffer), 0);
 	t_mensaje* mensaje = deserializarMensaje(buffer, nbytes);
 	return mensaje;
 }

/*
 * Deserializar Mensaje.
 *
 */
 t_mensaje* deserializarMensaje(char* buffer, int nbytes)
 {
 	t_stream* stream = malloc(sizeof(t_stream));
 	stream->data = buffer;
 	stream->length = nbytes;

 	t_mensaje* mensaje = deserializador(stream);
 	return mensaje;
 }

/*
 * Serializacion
 *
 */
 t_stream *serializador(t_mensaje *mensaje)
 {
 	char *data = malloc(strlen(mensaje->info) + 1);
 	t_stream *stream = malloc(sizeof(t_stream));
 	int offset = 0, tmp_size = 0;
 	memcpy(data, &mensaje->id, tmp_size = sizeof(char));
 	offset = tmp_size;
 	memcpy(data + offset, mensaje->info, tmp_size = strlen(mensaje->info) + 1);
 	offset += tmp_size;
 	stream->length = offset;
 	stream->data = data;
 	return stream;
 }

/*
 * Deserializacion.
 *
 */
 t_mensaje *deserializador(t_stream *stream)
 {
 	t_mensaje *mensaje = malloc(sizeof(t_mensaje));
 	int offset = 0;
 	int tmp_size = 0;
 	memcpy(&mensaje->id, stream->data, tmp_size = sizeof(char));
 	offset = tmp_size;
 	for (tmp_size = 1; (stream->data + offset)[tmp_size - 1] != '\0';
 		tmp_size++);
 		mensaje->info = malloc(tmp_size);
 	memcpy(mensaje->info, stream->data + offset, tmp_size);
 	return mensaje;
 }

/*
 * Devuelve el numero mayor. 
 * 
 */
 int obtenerMayor(int mayor, int numero)
 {
 	return mayor < numero ? numero : mayor;
 }

/*
 * Convierte un numero entero a su equivalente caracter.
 * Devuelve dicho caracter.
 */
 char intToChar(int i)
 {
 	return (char)(((int)'0')+i);
 }

/*
 * Aplica rand en un intervalo 0 - limite.
 * Devuelve dicho numero.
 */
 int randomizarNumero(int limite)
 {
 	return rand() % (limite+1);
 }

/*
 * Genera un long que sirve de semilla para rand() segun el tiempo y pid.
 * Devuelve la semilla.
 */
 long seedgen()
 {
 	long s, seed, pid;
 	time_t seconds;
 	pid = getpid();
    s = time ( &seconds ); /* get CPU seconds since 01/01/1970 */
 	seed = abs(((s*181)*((pid-83)*359))%104729);
 	return seed;
 }

/*
 * Duerme un proceso en milisegundos.
 */
 extern int msleep(__useconds_t __useconds)
 {
 	return usleep(__useconds*1000);
 }
