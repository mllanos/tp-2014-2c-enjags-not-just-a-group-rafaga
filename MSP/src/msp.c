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
	int listener, nuevaConexion;

	/* Creación de archivo log */
	Logger = log_create(LOG_PATH, "MSP", false, LOG_LEVEL_TRACE);

	cargarConfiguracion(argv[1]);
	inicializarMSP();

	log_trace(Logger, "Inicio de MSP.\n	Tamaño de Memoria Principal: %u.\n	Tamaño de SWAP: %u.", MaxMem, MaxSwap);

	listener = server_socket(Puerto);
	pthread_create(&thread, NULL, atenderConsola, NULL);

	while (true) {

		nuevaConexion = accept_connection(listener);

		pthread_mutex_lock(&LogMutex);
		log_trace(Logger, "Nueva conexión.");
		pthread_mutex_unlock(&LogMutex);

		pthread_create(&thread, NULL, atenderProceso, &nuevaConexion);
	}

	return EXIT_SUCCESS;

}
