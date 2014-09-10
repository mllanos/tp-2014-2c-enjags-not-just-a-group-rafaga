#include "consola.h"

char *direccionIP;
int puerto;


int main (int argc, char *argv[]){

	//Levantar archivo de configuracion
	t_config* config;
	config=config_create(PATH_ARCHIVO_CONF);
	puerto=config_get_int_value(config,"PUERTO_KERNEL");
	direccionIP=config_get_string_value(config,"IP_KERNEL");
	//Fin levantar archivo de configuracion

	return 0;
}
