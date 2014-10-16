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
	char *error,*bytesAEscribir;
	int proceso = *((int*)parametro);
	uint32_t pid,size,direccionLogica,numeroSegmento;

	while(true) {

		msg = recibir_mensaje(proceso);

		switch(msg->header.id) {
		case WRITE_MEMORY:
			pthread_mutex_lock(&LogMutex);
			log_trace(Logger,"Recepción de solicitud Escribir Memoria\nPID: %d\nDirección Lógica: %d\nBytes a Escribir: %s\nTamaño: %d",pid,direccionLogica,bytesAEscribir,size);
			pthread_mutex_unlock(&LogMutex);
			pid = msg->argv[0];
			direccionLogica = msg->argv[1];
			bytesAEscribir = msg->stream;
			size = msg->argv[2];
			pthread_mutex_lock(&MemMutex);
			msg->header.id = escribirMemoria(pid,direccionLogica,bytesAEscribir,size);
			pthread_mutex_unlock(&MemMutex);
			free(msg->stream);
			msg->header.length = 0;
			break;
		case CREATE_SEGMENT:
			pthread_mutex_lock(&LogMutex);
			log_trace(Logger,"Recepción de solicitud Crear Segmento\nPID: %d\nTamaño: %d",pid,size);
			pthread_mutex_unlock(&LogMutex);
			pid = msg->argv[0];
			size = msg->argv[1];
			crearSegmento(pid,size,&msg->header.id);
			if(msg->header.id == OK_CREATE) {
				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Segmento %d del proceso %d creado correctamente",pid,numeroSegmento);
				pthread_mutex_unlock(&LogMutex);
			}
			else {
				error = id_string(msg->header.id);
				pthread_mutex_lock(&LogMutex);
				log_error(Logger,"No se pudo crear el segmento %d del proceso %d: %s",pid,numeroSegmento,error);
				pthread_mutex_unlock(&LogMutex);
				free(error);
			}
			break;
		case DESTROY_SEGMENT:
			pthread_mutex_lock(&LogMutex);
			log_trace(Logger,"Recepción de solicitud Destruir Segmento\nPID: %d\nNúmero de Segmento: %d",pid,numeroSegmento);
			pthread_mutex_unlock(&LogMutex);
			pid = msg->argv[0];
			numeroSegmento = msg->argv[1];
			if((msg->header.id = destruirSegmento(pid,numeroSegmento)) == OK_DESTROY) {
				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Segmento %d del proceso %d destruido correctamente",pid,numeroSegmento);
				pthread_mutex_unlock(&LogMutex);
			}
			else {
				pthread_mutex_lock(&LogMutex);
				log_error(Logger,"No se pudo destruir el segmento %d del proceso %d",pid,numeroSegmento);
				pthread_mutex_unlock(&LogMutex);
			}
			break;
		case REQUEST_MEMORY:
			pthread_mutex_lock(&LogMutex);
			log_trace(Logger,"Recepción de solicitud Escribir Memoria\nPID: %d\nDirección Lógica: %d\nTamaño: %d",pid,direccionLogica,size);
			pthread_mutex_unlock(&LogMutex);
			pid = msg->argv[0];
			direccionLogica = msg->argv[1];
			size = msg->argv[2];
			msg->stream = solicitarMemoria(pid,direccionLogica,size,&msg->header.id);
			msg->header.length = size;
			break;
		default:
			pthread_mutex_lock(&LogMutex);
			log_error(Logger,"Recepción de solicitud inválida");
			pthread_mutex_unlock(&LogMutex);
		}

		free(msg->argv);
		msg->header.argc = 0;

		enviar_mensaje(proceso,msg);

		destroy_message(msg);
	}

	return NULL;
}
