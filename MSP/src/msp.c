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
	Logger = log_create(argv[2],"MSP",false,LOG_LEVEL_TRACE);

	cargarConfiguracion(argv[1]);
	inicializarMSP(argv[3]);

	log_trace(Logger,"Inicio de MSP\nTamaño de página: %d\nTamaño de swap: %d",MaxMem,MaxSwap);

	listener = server_socket(Puerto);
	pthread_create(&thread,NULL,atenderConsola,NULL);

	while(true) {

		nuevaConexion = accept_connection(listener);
		log_trace(Logger,"Nueva conexión");
		pthread_create(&thread,NULL,atenderProceso,&nuevaConexion);

	}

	return EXIT_SUCCESS;

}