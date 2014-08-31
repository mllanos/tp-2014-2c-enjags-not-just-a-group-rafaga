#include "utiles.h"
#include "socket.h"

/*
 * Crear socket
 *
 */
int crearSocket(char* ip, int puerto){
	int sockfd;
	struct sockaddr_in dest_addr;
	socket_crear(&sockfd);
	socket_info_dest_direc(&dest_addr, puerto, ip);
	socket_conectar(sockfd, &dest_addr);
	return sockfd;
}

/*
 * Enviar Mensaje
 *
 */
void enviarMensaje(int sockfd, char* info){
        t_mensaje* mensaje = malloc(sizeof(t_mensaje));
        mensaje->id = '_';
        mensaje->info = info;
    	t_stream* stream = serializador(mensaje);
        socket_enviar(sockfd, stream->data, stream->length);
}

/*
 * Recibir Mensaje
 *
 */
t_mensaje* recibirMensaje(int sockfd){
    	char buffer[256];
    	int nbytes = recv(sockfd, buffer, sizeof(buffer), 0);
    	t_mensaje* mensaje = descerializarMensaje(buffer, nbytes);
    	return mensaje;
}

/*
 * Descerializar Mensaje
 *
 */
t_mensaje* descerializarMensaje(char* buffer, int nbytes){
    	t_stream* stream = malloc(sizeof(t_stream));
    	stream->data = buffer;
    	stream->length = nbytes;

    	t_mensaje* mensaje = descerializador(stream);
    	return mensaje;
}

/*
 * Serializacion
 *
 */
t_stream *serializador(t_mensaje *mensaje) {
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
 * Descerializacion
 *
 */
t_mensaje *descerializador(t_stream *stream) {
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
int obtenerMayor(int mayor, int numero){
    if(mayor < numero){
        mayor = numero;
    }
    return mayor;
}

/*
 * Convierte un numero entero a su equivalente caracter.
 * Devuelve dicho caracter.
 */
char intToChar(int i) {
	return (char)(((int)'0')+i);
}

/*
 * Aplica rand en un intervalo 0 - limite.
 * Devuelve dicho numero.
 */
int randomizarNumero(int limite) {
	return rand() % (limite+1);
}

/*
 * Genera un long que sirve de semilla para rand() segun el tiempo y pid.
 * Devuelve la semilla.
 */
long seedgen()  {
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
extern int msleep(__useconds_t __useconds) {
	return usleep(__useconds*1000);
}
