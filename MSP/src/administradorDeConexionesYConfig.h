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

/* el archivo configuracion será del tipo:
 * PUERTO=[1024-65535]
 * CANTIDAD_MEMORIA=[KB]
 * CANTIDAD_SWAP=[MB]
 * SUST_PAGS=LRU/CLOCK
 */
void cargarConfiguracion(char* path);
void *atenderProceso(void* parametro);
void *atenderConsola(void* parametro);

#endif /* ADMINISTRADORDECONEXIONESYCONFIG_H_ */
