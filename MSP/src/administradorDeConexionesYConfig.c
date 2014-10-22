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

	uint32_t i;
	t_msg_id msg_id;
	t_segmento *tabla;
	uint16_t seg,pag,cantPag;
	uint32_t pid,size,direccionLogica,baseSegmento;
	char *error,*buffer,*stringPID,*parameters,aux[1];

	puts("Inicio de Consola MSP. A la espera de comandos...\n");

	while(true) {

		switch(esperarComando()) {
		case ESCRIBIR_MEMORIA:
			if(scanf("%m[^\n]",&parameters) == 0 || sscanf(parameters,"%u%u%u%*[ \t]%m[^\n]",&pid,&direccionLogica,&size,&buffer) != 4) {
				puts("Argumentos inválidos");
				free(parameters);
				break;
			}

			free(parameters);

			pthread_mutex_lock(&MemMutex);
			msg_id = escribirMemoria(pid,direccionLogica,buffer,size);
			pthread_mutex_unlock(&MemMutex);

			if( msg_id == SEGMENTATION_FAULT)
				puts("ERROR: SEGMENTATION FAULT.");

			free(buffer);
			break;
		case CREAR_SEGMENTO:
			if(scanf("%m[^\n]",&parameters) == 0 || sscanf(parameters,"%u%u%1s",&pid,&size,aux) != 2) {
				puts("Argumentos inválidos");
				free(parameters);
				break;
			}

			free(parameters);

			pthread_mutex_lock(&MemMutex);
			direccionLogica = crearSegmento(pid,size,&msg_id);
			pthread_mutex_unlock(&MemMutex);

			pthread_mutex_lock(&LogMutex);
			if(msg_id == OK_CREATE) {
				log_trace(Logger,"Segmento %u del proceso %u creado correctamente.",segmento(direccionLogica),pid);
				pthread_mutex_unlock(&LogMutex);

				printf("Dirección base del segmento %u del proceso %u: %u.\n",segmento(direccionLogica),pid,direccionLogica);
			}
			else {
				error = id_string(msg_id);
				log_error(Logger,"No se pudo crear el segmento %u del proceso %u: %s.",segmento(direccionLogica),pid,error);
				pthread_mutex_unlock(&LogMutex);

				printf("ERROR: %s.\n",error);

				//free(error); por alguna razon no puedo liberar este espacio de memoria. Pasa lo mismo con el string Algoritmo de config.
			}
			break;
		case DESTRUIR_SEGMENTO:
			if(scanf("%m[^\n]",&parameters) == 0 || sscanf(parameters,"%u%u%1s",&pid,&baseSegmento,aux) != 2) {
				puts("Argumentos inválidos");
				free(parameters);
				break;
			}

			free(parameters);

			pthread_mutex_lock(&MemMutex);
			msg_id = destruirSegmento(pid,baseSegmento);
			pthread_mutex_unlock(&MemMutex);

			pthread_mutex_lock(&LogMutex);
			if(msg_id == OK_DESTROY) {
				log_trace(Logger,"Segmento %u del proceso %u destruido correctamente.",segmento(baseSegmento),pid);
				pthread_mutex_unlock(&LogMutex);

				printf("Segmento %u del proceso %u destruido correctamente.\n",segmento(baseSegmento),pid);
			}
			else {
				log_error(Logger,"No se pudo destruir el segmento %u del proceso %u.",segmento(baseSegmento),pid);
				pthread_mutex_unlock(&LogMutex);

				printf("No se pudo destruir el segmento %u del proceso %u.\n",segmento(baseSegmento),pid);
			}
			break;
		case LEER_MEMORIA:
			if(scanf("%m[^\n]",&parameters) == 0 || sscanf(parameters,"%u%u%u%1s",&pid,&direccionLogica,&size,aux) != 3) {
				puts("Argumentos inválidos");
				free(parameters);
				break;
			}

			free(parameters);

			pthread_mutex_lock(&MemMutex);
			buffer = solicitarMemoria(pid,direccionLogica,size,&msg_id);
			pthread_mutex_unlock(&MemMutex);

			if(msg_id == SEGMENTATION_FAULT)
				puts("ERROR: SEGMENTATION FAULT.");
			else
				printf("%.*s\n",size,buffer);
			break;
		case TABLA_SEGMENTOS:
			printf("%-14s%-14s %-14s %-s\n","PID","Nº Segmento","Tamaño","Dirección Base");

			pthread_mutex_lock(&LogMutex);
			log_trace(Logger,"%-14s%-14s %-14s %-s","PID","Nº Segmento","Tamaño","Dirección Base");
			pthread_mutex_unlock(&LogMutex);

			dictionary_iterator(TablaSegmentosGlobal,imprimirSegmento);
			break;
		case TABLA_PAGINAS:
			if(scanf("%m[^\n]",&parameters) == 0 || sscanf(parameters,"%u%1s",&pid,aux) != 1) {
				puts("Argumentos inválidos");
				free(parameters);
				break;
			}

			free(parameters);

			if((tabla = dictionary_get(TablaSegmentosGlobal,stringPID = string_uitoa(pid))) == NULL) {
				free(stringPID);
				puts("PID INVÁLIDO");
				break;
			}

			free(stringPID);

			printf("%-14s%-14s%-s\n","Nº Segmento","Nº Página","En Memoria");

			pthread_mutex_lock(&LogMutex);
			log_trace(Logger,"%-14s%-14s%-s","Nº Segmento","Nº Página","En Memoria");
			pthread_mutex_unlock(&LogMutex);

			for(seg=0;seg < NUM_SEG_MAX;++seg)
				if(segmentoValido(tabla,seg)) {
					cantPag = cantidadPaginasTotalDelSegmento(tabla,seg);

					for(pag=0;pag < cantPag;++pag) {
						pthread_mutex_lock(&LogMutex);
						log_trace(Logger,"%-13u%-12u%-u",seg,pag,tabla[seg].tablaPaginas[pag].bitPresencia);
						pthread_mutex_unlock(&LogMutex);

						printf("%-13u%-12u%-u\n",seg,pag,tabla[seg].tablaPaginas[pag].bitPresencia);
					}
				}
			break;
		case LISTAR_MARCOS:
			printf("%-14s%-14s%-s\n","Nº Marco","Ocupado","PID");

			pthread_mutex_lock(&LogMutex);
			log_trace(Logger,"%-14s%-14s%-s","Nº Marco","Ocupado","PID");
			pthread_mutex_unlock(&LogMutex);

			for(i=0;i < CantidadMarcosTotal;++i) {
				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"%-13u%-14u%-u",i,MemoriaPrincipal[i].ocupado,MemoriaPrincipal[i].pid);
				pthread_mutex_unlock(&LogMutex);

				printf("%-13u%-14u%-u\n",i,MemoriaPrincipal[i].ocupado,MemoriaPrincipal[i].pid);
			}

			ImprimirInfoAlgoritmosSustitucion();

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
				log_trace(Logger,"Recepción de solicitud Escribir Memoria\nPID: %u\nDirección Lógica: %u\nBytes a Escribir: %s\nTamaño: %u",pid,direccionLogica,bytesAEscribir,size);
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
				log_trace(Logger,"Recepción de solicitud Crear Segmento\nPID: %u\nTamaño: %u",pid,size);
				pthread_mutex_unlock(&LogMutex);

				pthread_mutex_lock(&MemMutex);
				msg->argv[0] = direccionLogica = crearSegmento(pid,size,&msg->header.id);
				pthread_mutex_unlock(&MemMutex);
				msg->header.argc = 1;

				pthread_mutex_lock(&LogMutex);
				if(msg->header.id == OK_CREATE) {
					log_trace(Logger,"Segmento %u del proceso %u creado correctamente",pid,segmento(direccionLogica));
					pthread_mutex_unlock(&LogMutex);
				}
				else {
					error = id_string(msg->header.id);

					log_error(Logger,"No se pudo crear el segmento %u del proceso %u: %s",pid,segmento(direccionLogica),error);
					pthread_mutex_unlock(&LogMutex);

					//free(error); por alguna razon no puedo liberar este espacio de memoria. Pasa lo mismo con el string Algoritmo de config.
				}

				enviar_mensaje(proceso,msg);
				break;
			case DESTROY_SEGMENT:
				pid = msg->argv[0];
				baseSegmento = msg->argv[1];

				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Recepción de solicitud Destruir Segmento\nPID: %u\nNúmero de Segmento: %u",pid,segmento(baseSegmento));
				pthread_mutex_unlock(&LogMutex);

				pthread_mutex_lock(&MemMutex);
				msg->header.id = destruirSegmento(pid,baseSegmento);
				pthread_mutex_unlock(&MemMutex);

				pthread_mutex_lock(&LogMutex);
				if(msg->header.id == OK_DESTROY) {
					log_trace(Logger,"Segmento %u del proceso %u destruido correctamente",pid,segmento(baseSegmento));
					pthread_mutex_unlock(&LogMutex);
				}
				else {
					log_error(Logger,"No se pudo destruir el segmento %u del proceso %u",pid,segmento(baseSegmento));
					pthread_mutex_unlock(&LogMutex);
				}

				free(msg->argv);
				msg->header.argc = 0;

				enviar_mensaje(proceso,msg);
				break;
			case REQUEST_MEMORY:
				pthread_mutex_lock(&LogMutex);
				log_trace(Logger,"Recepción de solicitud Escribir Memoria\nPID: %u\nDirección Lógica: %u\nTamaño: %u",pid,direccionLogica,size);
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
		scanf("%*[^\n]");
	}

	free(command);

	return idCommand;

}

void imprimirSegmento(char *pid,void *data) {

	int seg;
	t_segmento *tabla = (t_segmento*) data;

	for(seg=0;seg < NUM_SEG_MAX;++seg)
		if(segmentoValido(tabla,seg)) {
			pthread_mutex_lock(&LogMutex);
			log_trace(Logger,"%-14s%-14u%-14u%-u",pid,seg,tabla[seg].limite,generarDireccionLogica(seg,0,0));
			pthread_mutex_unlock(&LogMutex);

			printf("%-14s%-14u%-14u%-u\n",pid,seg,tabla[seg].limite,generarDireccionLogica(seg,0,0));
		}
}
