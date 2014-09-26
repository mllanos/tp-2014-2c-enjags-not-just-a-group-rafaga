#include "consola.h"

char *direccionIP;
char * puerto;
int sizeArchivo,leido;
int socketDesc;

char* leerUnBESO (char *rutaArchivo, int *tamanioBuffer) {

	FILE *archivoBESO;
	char *buffer;

	archivoBESO=fopen(rutaArchivo,"r");

	if(archivoBESO) {
		fseek(archivoBESO,0,SEEK_END); // me posiciono al final
		sizeArchivo=ftell(archivoBESO);
		*tamanioBuffer = sizeArchivo;
		rewind(archivoBESO); // volver al inicio del archivo
		buffer=malloc(sizeof(char)*sizeArchivo);
		leido=fread(buffer,sizeof(char),sizeArchivo,archivoBESO);
		fclose(archivoBESO);
	} else {
		printf("Error al abrir archivo");
	}
	if (leido != sizeArchivo){ // no se leyo completamente
		free(buffer);
		buffer=NULL;
	}
	return buffer;
}

t_log *logger; //Archivo de log
char path_log[30] = "log/logConsola"; //Path del log

int main (int argc, char *argv[]){

	int tamanioBuffer = 0;
	char* mensaje;
	logger = log_create(path_log, "Consola",1, LOG_LEVEL_TRACE);
	log_trace(logger, "BIENVENIDO AL PROCESO CONSOLA");
	//Levantar archivo de configuracion
	t_config* config;
	config=config_create(PATH_ARCHIVO_CONF);
	puerto=config_get_string_value(config,"PUERTO_KERNEL");
	direccionIP=config_get_string_value(config,"IP_KERNEL");
	//Fin levantar archivo de configuracion

	mensaje=leerUnBESO("/home/utnso/git/tp-2014-2c-enjags-not-just-a-group/Ensamblador/EjemplosESO/bigStack.txt",&tamanioBuffer);

	socketDesc=socket_cliente(direccionIP,puerto);

	send(socketDesc, mensaje, sizeArchivo + 1, 0);

	if (socketDesc<0) {
		log_trace(logger,"ERROR AL CONECTAR AL KERNEL, FINALIZANDO PROGRAMA");
		exit(-1);
	}

	return 0;
}
