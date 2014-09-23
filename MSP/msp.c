
#include "umv.h"
#include <stdlib.h>

#define STDIN            0
#define TAM_BUFF_COMANDO 128

char ip[16], algoritmo[8];
int puerto, cmemoria, cswap;
t_log  *logfile;


int main (int argc, char** argv){
	int socketActivo,socketEscucha;

	//Nos loguemos (utilizando la libreria commons)
	loguer=crearLog(argv[0]);
	//Cargamos el archivo de configuracion
	cargarConficuracion(argv[1]);





	return 0;
}

t_log *crearLog(char *archivo){
	char path[11]={0};
	char aux[17]={0};

	strcpy(path,"logueo.log");
	strcat(aux,"touch ");
	strcat(aux,path);
	system(aux);
	t_log *logAux=log_create(path,archivo,false,LOG_LEVEL_DEBUG);
	log_info(logAux,"ARCHIVO DE LOG CREADO");
	return logAux;
}



void cargarConficuracion(char* path){
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
	//printf("archivo de configuracion levantado: ip:%s ...);
	log_debug(logfile,"cargarConficuracion(char* path) exitoso, con ip:%s puerto:%i tamanio de memoria:%i swap:%i algoritmo:%i ...",ip,puerto,cmemoria,cswap,algoritmo);
}
