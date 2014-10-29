/*
 * administradorDeConexionesYConfig.h
 *
 *  Created on: 12/10/2014
 *      Author: matias
 */

#ifndef ADMINISTRADORDECONEXIONESYCONFIG_H_
#define ADMINISTRADORDECONEXIONESYCONFIG_H_

#include "msp.h"
#include "administradorDeMemoria.h"

#define QUERY_PARAMS "%m[^\n]"

typedef enum {
	CREAR_SEGMENTO,
	DESTRUIR_SEGMENTO,
	ESCRIBIR_MEMORIA,
	LEER_MEMORIA,
	TABLA_SEGMENTOS,
	TABLA_PAGINAS,
	LISTAR_MARCOS, 
	COMANDO_INVALIDO, 
	SWAP,
	CLEAR,
	CLEAR_SWAP,
	HELP,
	QUIT
} t_comando_consola;

/* el archivo configuracion será del tipo:
 * PUERTO=[1024-65535]
 * CANTIDAD_MEMORIA=[KB]
 * CANTIDAD_SWAP=[MB]
 * SUST_PAGS=LRU/CLOCK
 */
void cargarConfiguracion(char* path);
void *atenderProceso(void* parametro);
void *atenderConsola(void* parametro);
t_comando_consola esperarComando(void);
void imprimirSegmento(char *pid, void *data);

#endif /* ADMINISTRADORDECONEXIONESYCONFIG_H_ */
