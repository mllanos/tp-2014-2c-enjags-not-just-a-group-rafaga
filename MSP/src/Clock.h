/*
 * Clock.h
 *
 *  Created on: 13/10/2014
 *      Author: matias
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include "administradorDeMemoria.h"

typedef struct {
	uint32_t pid;
	uint16_t numSegmento;
	uint16_t numPagina;
	bit bitReferencia;
} __attribute__ ((__packed__)) t_clock_node;

/* Variables Globales */
uint32_t ClockIndex;				/* Almacena la posición de la última página swappeada */
uint32_t ClockEmptyIndex;			/* Almacena la posición del primer registro vacío del array */
t_clock_node *ArrayClock;			/* Estructura de control para el algoritmo Clock */
/* FIN Variables Globales */

/* Funciones */
void imprimirArrayClock(void);
void actualizarEnArrayClock(uint32_t pid, uint16_t seg, uint16_t pag);
void quitarPaginaDeArrayClock(uint32_t pid, uint16_t seg, uint16_t pag);
void agregarPaginaEnArrayClock(uint32_t pid, uint16_t seg, uint16_t pag);
void seleccionVictimaClock(t_segmento** tabla, uint32_t *pid, uint16_t *seg, uint16_t *pag);
/* FIN Funciones */

#endif /* CLOCK_H_ */
