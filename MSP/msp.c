
#include "umv.h"
#include <stdlib.h>

#define STDIN            0
#define TAM_BUFF_COMANDO 128

char ip[16], algoritmo[8];
int puerto, cmemoria, cswap;


int main (int argc, char** argv){
	int socketActivo,socketEscucha;

	//Nos loguemos (utilizando la libreria commons)
	loguer=crearLog(argv[0]);






	return 0;
}

void levantarArchivoConfig(char* path){
	/* el archivo configuracion sera del tipo:
	 * IP=127.0.0.1
	 * PUERTO=5000
	 * CANTIDAD_MEMORIA=KB
	 * CANTIDAD_SWAP=MB
	 * SUST_PAGS=LRU/CLOCK
	 */
	char          *ipconfig;
	t_config      *configUMV;
	extern t_log  *loguer;

	configUMV=config_create(path);
	ipconfig      =config_get_string_value(configUMV,"IP");
	memcpy(ip,ipconfig,strlen(ip)+1);
	puerto 		  =config_get_int_value(configUMV,"PUERTO");
	cmemoria      =config_get_int_value(configUMV,"CANTIDAD_MEMORIA=");
	cswap         =config_get_int_value(configUMV,"CANTIDAD_SWAP");
	algoritmo     =config_get_string_value(configUMV,"SUST_PAGS");
	config_destroy(configUMV);
	//printf("archivo de configuracion levantado: ip:%s puerto:%i tamanio de bloque:%i\n",ipUMV,puertoUMV,tamanioBloque);
	log_debug(loguer,"levantarArchivoConf()==>Se levanto el archivo con ip:%s puerto:%i tamanio de memoria:%i swap:%i algoritmo:%i ...",ip,puerto,cmemoria,cswap,algoritmo);
}
