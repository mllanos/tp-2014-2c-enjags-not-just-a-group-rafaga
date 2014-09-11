#include "consola.h"

char *direccionIP;
int puerto;
int sizeArchivo,leido;

char* leerUnESO (char *rutaArchivo) {

	FILE *archivoESO;
	char *buffer;

	archivoESO=fopen(rutaArchivo,"r");

	if(archivoESO) {
		fseek(archivoESO,0,SEEK_END); // me posiciono al final
		sizeArchivo=ftell(archivoESO);
		rewind(archivoESO); // volver al inicio del archivo
		buffer=malloc(sizeof(char)*sizeArchivo);
		leido=fread(buffer,sizeof(char),sizeArchivo,archivoESO);
		fclose(archivoESO);
	} else {
		printf("Error al abrir archivo");
	}
	if (leido != sizeArchivo){ // no se leyo completamente
		free(buffer);
		buffer=NULL;
	}
	return buffer;
}


int main (int argc, char *argv[]){

	//Levantar archivo de configuracion
	t_config* config;
	config=config_create(PATH_ARCHIVO_CONF);
	puerto=config_get_int_value(config,"PUERTO_KERNEL");
	direccionIP=config_get_string_value(config,"IP_KERNEL");
	//Fin levantar archivo de configuracion

	leerUnESO("/home/utnso/git/tp-2014-2c-enjags-not-just-a-group/Ensamblador/EjemplosESO/bigStack.txt");


	return 0;
}
