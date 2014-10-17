/*
 * msp.c
 *
 *  Created on: 09/10/2014
 *      Author: matias
 */

#include "msp.h"
#include "administradorDeConexionesYConfig.h"

int main (int argc, char** argv) {

	pthread_t thread;
	int listener,nuevaConexion;

	/* Creación de archivo log */
	clean_file(argv[2]);
	Logger = log_create(argv[2],"MSP",false,LOG_LEVEL_TRACE);

	cargarConfiguracion(argv[1]);
	inicializarMSP(argv[3]);

	log_trace(Logger,"Inicio de MSP.\n	Tamaño de página: %d.\n	Tamaño de swap: %d.",MaxMem,MaxSwap);

	listener = server_socket(Puerto);
	pthread_create(&thread,NULL,atenderConsola,NULL);

	while(true) {

		nuevaConexion = accept_connection(listener);

		pthread_mutex_lock(&LogMutex);
		log_trace(Logger,"Nueva conexión.");
		pthread_mutex_unlock(&LogMutex);

		pthread_create(&thread,NULL,atenderProceso,&nuevaConexion);

	}

	return EXIT_SUCCESS;

}
