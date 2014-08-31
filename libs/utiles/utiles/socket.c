#include "socket.h"

void socket_crear(int* sockfd){
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("error al crear socket");
		exit(1);
	}
}

void socket_setopt(int sockfd, int* yes){
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
}

void socket_info_mi_direc(struct sockaddr_in* mi_direc, int puerto){
	mi_direc->sin_family = AF_INET;
	mi_direc->sin_addr.s_addr = INADDR_ANY;
	mi_direc->sin_port = htons(puerto);
	memset(&(mi_direc->sin_zero), '\0', 8);
}

void socket_bind(int sockfd, struct sockaddr_in* mi_direc){
	if (bind(sockfd, (struct sockaddr*) mi_direc, sizeof(*mi_direc)) == -1) {
		perror("bind");
		exit(1);
	}
}

void socket_escuchar(int sockfd){
	if (listen(sockfd, 0) == -1) {
		perror("listen");
		exit(1);
	}
}

int socket_aceptar(int sockfd, struct sockaddr_in* p_dest_direc, socklen_t * p_addrlen ){
	int socketNuevaConexion;
	if ((socketNuevaConexion = accept(sockfd,(struct sockaddr*) p_dest_direc, p_addrlen)) == -1)
		perror("accept");
	return socketNuevaConexion;
}

void socket_info_dest_direc(struct sockaddr_in* dest_direc, int puerto, char* ip){
	dest_direc->sin_family = AF_INET;
	dest_direc->sin_addr.s_addr = inet_addr(ip);
	dest_direc->sin_port = htons(puerto);
	memset(&(dest_direc->sin_zero), '\0', 8);
}

void socket_conectar(int sockfd, struct sockaddr_in* p_dest_direc){
	if(connect(sockfd, (struct sockaddr *)p_dest_direc, sizeof(struct sockaddr)) == -1){
		perror("Error al conectar socket");
		exit(1);
	}
}

int socket_recibir(int unSocket, char* buffer){
	int nbytes;
	if ((nbytes = recv(unSocket, buffer, sizeof(buffer), 0)) < 0)
		perror("error en la recepcion");
	return nbytes;
}

int socket_enviar(int unSocket, char* data, int length){
	int nbytes;
	if((nbytes=send(unSocket, data, length ,0)) <0)
		perror("Error en el send");
	return nbytes;
}

void socket_cerrar(int socket){
	close(socket);
}
