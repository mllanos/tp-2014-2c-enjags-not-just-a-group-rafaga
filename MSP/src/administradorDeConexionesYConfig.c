/*
 * administradorDeConexiones.c
 *
 *  Created on: 09/10/2014
 *      Author: matias
 */

#include "administradorDeConexionesYConfig.h"

void cargarConfiguracion(char* path) {

	t_config* config = config_create(path);
	Puerto = config_get_int_value(config,"PUERTO");
	MaxMem = config_get_int_value(config,"CANTIDAD_MEMORIA")*K;
	MaxSwap = config_get_int_value(config,"CANTIDAD_SWAP")*M;
	AlgoritmoSustitucion = config_get_string_value(config,"SUST_PAGS");
	config_destroy(config);

}

void *atenderConsola(void* parametro) {

	return NULL;

}

void *atenderProceso(void* parametro) {

	t_msg *msg;
	char* bytesAEscribir;
	int proceso = *((int*)parametro);
	uint32_t pid,size,direccionLogica,numeroSegmento;

	while(true) {

		msg = recibir_mensaje(proceso);

		switch(msg->header.id) {
		case WRITE_MEMORY:
			log_trace(Logger,"Recepción de solicitud Escribir Memoria\nPID: %d\nDirección Lógica: %d\nBytes a Escribir: %s\nTamaño: %d",pid,direccionLogica,bytesAEscribir,size);
			pid = msg->argv[0];
			direccionLogica = msg->argv[1];
			bytesAEscribir = msg->stream;
			size = msg->argv[2];
			msg->header.id = escribirMemoria(pid,direccionLogica,bytesAEscribir,size);
			free(msg->stream);
			msg->header.length = 0;
			break;
		case CREATE_SEGMENT:
			log_trace(Logger,"Recepción de solicitud Crear Segmento\nPID: %d\nTamaño: %d",pid,size);
			pid = msg->argv[0];
			size = msg->argv[1];
			crearSegmento(pid,size,&msg->header.id);
			break;
		case DESTROY_SEGMENT:
			log_trace(Logger,"Recepción de solicitud Destruir Segmento\nPID: %d\nNúmero de Segmento: %d",pid,numeroSegmento);
			pid = msg->argv[0];
			numeroSegmento = msg->argv[1];
			msg->header.id = destruirSegmento(pid,numeroSegmento);
			break;
		case REQUEST_MEMORY:
			log_trace(Logger,"Recepción de solicitud Escribir Memoria\nPID: %d\nDirección Lógica: %d\nTamaño: %d",pid,direccionLogica,size);
			pid = msg->argv[0];
			direccionLogica = msg->argv[1];
			size = msg->argv[2];
			msg->stream = solicitarMemoria(pid,direccionLogica,size,&msg->header.id);
			msg->header.length = size;
			break;
		default:
			log_error(Logger,"Solicitud inválida recibida");
		}

		free(msg->argv);
		msg->header.argc = 0;

		enviar_mensaje(proceso,msg);

		destroy_message(msg);
	}

	return NULL;
}
