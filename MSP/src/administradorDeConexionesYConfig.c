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
	//config_destroy(config);
	//por alguna razon no puedo accede al espacio de memoria de AlgoritmoSustitucion. Pasa lo mismo con el string error en atender consola.

}

void *atenderConsola(void* parametro) {

	t_msg_id msg_id;
	char *error,*bytesAEscribir;//buffer
	uint32_t pid,size,direccionLogica,baseSegmento;

	puts("Inicio de Consola MSP. A la espera de comandos...\n");

	while(true) {

		switch(esperarComando()) {
		case ESCRIBIR_MEMORIA:
			scanf("%u%u%u",&pid,&direccionLogica,&size);
			scanf("%*[ \t]%m[^\n]",&bytesAEscribir);

			pthread_mutex_lock(&MemMutex);
			msg_id = escribirMemoria(pid,direccionLogica,bytesAEscribir+1,size);
			pthread_mutex_unlock(&MemMutex);

			if( msg_id == SEGMENTATION_FAULT)
				puts("ERROR: SEGMENTATION FAULT.");

			free(bytesAEscribir);
			break;
		case CREAR_SEGMENTO:
			scanf("%u%u",&pid,&size);

			pthread_mutex_lock(&MemMutex);
			direccionLogica = crearSegmento(pid,size,&msg_id);
			pthread_mutex_unlock(&MemMutex);

			pthread_mutex_lock(&LogMutex);
			if(msg_id == OK_CREATE) {
				//pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Segmento %d del proceso %d creado correctamente.",segmento(direccionLogica),pid);
				pthread_mutex_unlock(&LogMutex);

				printf("Dirección base del segmento %u del proceso %u: %u.\n",segmento(direccionLogica),pid,direccionLogica);
			}
			else {
				error = id_string(msg_id);
				//pthread_mutex_lock(&LogMutex);
				log_error(Logger,"No se pudo crear el segmento %d del proceso %u: %s.",segmento(direccionLogica),pid,error);
				pthread_mutex_unlock(&LogMutex);

				printf("ERROR: %s.\n",error);

				//free(error); por alguna razon no puedo liberar este espacio de memoria. Pasa lo mismo con el string Algoritmo de config.
			}
			break;
		case DESTRUIR_SEGMENTO:
			scanf("%u%u",&pid,&baseSegmento);

			pthread_mutex_lock(&MemMutex);
			msg_id = destruirSegmento(pid,baseSegmento);
			pthread_mutex_unlock(&MemMutex);

			pthread_mutex_lock(&LogMutex);
			if(msg_id == OK_DESTROY) {
				//pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Segmento %u del proceso %u destruido correctamente.",segmento(baseSegmento),pid);
				pthread_mutex_unlock(&LogMutex);
			}
			else {
				//pthread_mutex_lock(&LogMutex);
				log_error(Logger,"No se pudo destruir el segmento %u del proceso %u.",segmento(baseSegmento),pid);
				pthread_mutex_unlock(&LogMutex);
			}
			break;
		case LEER_MEMORIA:
			scanf("%u%u%u",&pid,&direccionLogica,&size);

			pthread_mutex_lock(&MemMutex);
			bytesAEscribir = solicitarMemoria(pid,direccionLogica,size,&msg_id);
			pthread_mutex_unlock(&MemMutex);

			if(msg_id == SEGMENTATION_FAULT)
				puts("ERROR: SEGMENTATION FAULT.");
			else
				printf("%.*s\n",size,bytesAEscribir);
			break;
		default:
			puts("COMANDO INVÁLIDO.");
		}

	}

	return NULL;

}

void *atenderProceso(void* parametro) {

	t_msg *msg;
	char *error,*bytesAEscribir;
	int proceso = *((int*)parametro);
	uint32_t pid,size,direccionLogica,baseSegmento;

	while(true) {

		if((msg = recibir_mensaje(proceso)) != NULL) {

			switch(msg->header.id) {
			case WRITE_MEMORY:
				pid = msg->argv[0];
				direccionLogica = msg->argv[1];
				bytesAEscribir = msg->stream;
				size = msg->argv[2];

				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Recepción de solicitud Escribir Memoria\nPID: %d\nDirección Lógica: %d\nBytes a Escribir: %s\nTamaño: %d",pid,direccionLogica,bytesAEscribir,size);
				pthread_mutex_unlock(&LogMutex);

				pthread_mutex_lock(&MemMutex);
				msg->header.id = escribirMemoria(pid,direccionLogica,bytesAEscribir,size);
				pthread_mutex_unlock(&MemMutex);

				free(msg->argv);
				free(msg->stream);
				msg->header.argc = 0;
				msg->header.length = 0;

				enviar_mensaje(proceso,msg);
				break;
			case CREATE_SEGMENT:
				pid = msg->argv[0];
				size = msg->argv[1];

				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Recepción de solicitud Crear Segmento\nPID: %d\nTamaño: %d",pid,size);
				pthread_mutex_unlock(&LogMutex);

				pthread_mutex_lock(&MemMutex);
				msg->argv[0] = direccionLogica = crearSegmento(pid,size,&msg->header.id);
				pthread_mutex_unlock(&MemMutex);
				msg->header.argc = 1;

				pthread_mutex_lock(&LogMutex);
				if(msg->header.id == OK_CREATE) {
					//pthread_mutex_lock(&LogMutex);
					log_trace(Logger,"Segmento %d del proceso %d creado correctamente",pid,segmento(direccionLogica));
					pthread_mutex_unlock(&LogMutex);
				}
				else {
					error = id_string(msg->header.id);

					//pthread_mutex_lock(&LogMutex);
					log_error(Logger,"No se pudo crear el segmento %d del proceso %d: %s",pid,segmento(direccionLogica),error);
					pthread_mutex_unlock(&LogMutex);

					//free(error); por alguna razon no puedo liberar este espacio de memoria. Pasa lo mismo con el string Algoritmo de config.
				}

				enviar_mensaje(proceso,msg);
				break;
			case DESTROY_SEGMENT:
				pid = msg->argv[0];
				baseSegmento = msg->argv[1];

				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Recepción de solicitud Destruir Segmento\nPID: %d\nNúmero de Segmento: %d",pid,segmento(baseSegmento));
				pthread_mutex_unlock(&LogMutex);

				pthread_mutex_lock(&MemMutex);
				msg->header.id = destruirSegmento(pid,baseSegmento);
				pthread_mutex_unlock(&MemMutex);

				pthread_mutex_lock(&LogMutex);
				if(msg->header.id == OK_DESTROY) {
					//pthread_mutex_lock(&LogMutex);
					log_trace(Logger,"Segmento %d del proceso %d destruido correctamente",pid,segmento(baseSegmento));
					pthread_mutex_unlock(&LogMutex);
				}
				else {
					//pthread_mutex_lock(&LogMutex);
					log_error(Logger,"No se pudo destruir el segmento %d del proceso %d",pid,segmento(baseSegmento));
					pthread_mutex_unlock(&LogMutex);
				}

				free(msg->argv);
				msg->header.argc = 0;

				enviar_mensaje(proceso,msg);
				break;
			case REQUEST_MEMORY:
				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Recepción de solicitud Escribir Memoria\nPID: %d\nDirección Lógica: %d\nTamaño: %d",pid,direccionLogica,size);
				pthread_mutex_unlock(&LogMutex);

				pid = msg->argv[0];
				direccionLogica = msg->argv[1];
				size = msg->argv[2];

				pthread_mutex_lock(&MemMutex);
				msg->stream = solicitarMemoria(pid,direccionLogica,size,&msg->header.id);
				pthread_mutex_unlock(&MemMutex);

				free(msg->argv);
				msg->header.argc = 0;
				msg->header.length = msg->header.id == OK_REQUEST ? size : 0;

				enviar_mensaje(proceso,msg);
				break;
			default:
				pthread_mutex_lock(&LogMutex);
				log_error(Logger,"Recepción de solicitud inválida");
				pthread_mutex_unlock(&LogMutex);
			}
		}
		else {
			log_error(Logger,"Desconexión de un proceso.");
			break;
		}

		destroy_message(msg);
	}

	return NULL;
}

t_comando_consola esperarComando(void) {

	char *command;
	t_comando_consola idCommand;

	scanf("%ms",&command);

	if(!strcmp(command,"new"))
		idCommand = CREAR_SEGMENTO;
	else if(!strcmp(command,"destroy"))
		idCommand = DESTRUIR_SEGMENTO;
	else if(!strcmp(command,"write"))
		idCommand = ESCRIBIR_MEMORIA;
	else if(!strcmp(command,"read"))
		idCommand = LEER_MEMORIA;
	else if(!strcmp(command,"segtable"))
		idCommand = TABLA_SEGMENTOS;
	else if(!strcmp(command,"pagtable"))
		idCommand = TABLA_PAGINAS;
	else if(!strcmp(command,"frames"))
		idCommand = LISTAR_MARCOS;
	else {
		idCommand = COMANDO_INVALIDO;
		while(getchar() != EOF)
			;
	}

	free(command);

	return idCommand;

}
