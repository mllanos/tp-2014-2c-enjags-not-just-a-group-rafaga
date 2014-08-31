#include "error.h"

void errorHilo(int error){
	if (error) {
		perror("pthread_create");
	}
}

void errorPath(){
	log_error(logger, "Falta archivo de configuración");
}

void errorConexion(){
	log_error(logger, "Conexion incorrecta");
}

void errorNivel(char* nivel){
	log_error(logger, "Nivel no conectado; %s", nivel);
}

void errorAlgoritmo(char* algoritmo){
	log_error(logger, "Algoritmo incorrecto; %s", algoritmo);
}

void errorMensaje(char* mensaje){
	log_error(logger, "Mensaje incorrecto; %s", mensaje);
}
