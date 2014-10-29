/*
 * Clock.c
 *
 *  Created on: 13/10/2014
 *      Author: matias
 */

#include "Clock.h"

void seleccionVictimaClock(t_segmento** tabla, uint32_t *pid, uint16_t *seg, uint16_t *pag) {

	char *stringPID;
	uint32_t inPid = *pid;
	uint16_t inSeg = *seg;
	uint16_t inPag = *pag;

	for (; ArrayClock[ClockIndex = ClockIndex % ClockEmptyIndex].bitReferencia; ++ClockIndex)
		ArrayClock[ClockIndex].bitReferencia = 0;

	t_clock_node* pagMenosUsada = ArrayClock + ClockIndex;	/* Es equivalente a &ArrayClock[ClockIndex] */

	*pid = pagMenosUsada->pid;
	*pag = pagMenosUsada->numPagina;
	*seg = pagMenosUsada->numSegmento;
	*tabla = tablaDelProceso(stringPID = string_uitoa(*pid));

	free(stringPID);

	pagMenosUsada->pid = inPid;			/* Aprovecho y le cargo los datos de la nueva página, en el lugar de la que swapeé */
	pagMenosUsada->numPagina = inPag;
	pagMenosUsada->numSegmento = inSeg;
	pagMenosUsada->bitReferencia = 1;

}

void actualizarEnArrayClock(uint32_t pid, uint16_t seg, uint16_t pag) {

	int i;
	for (i = 0; ArrayClock[i].pid != pid || ArrayClock[i].numSegmento != seg || ArrayClock[i].numPagina != pag; ++i)
		;

	ArrayClock[i].bitReferencia = 1;

}

void agregarPaginaEnArrayClock(uint32_t pid, uint16_t seg, uint16_t pag) {

	ArrayClock[ClockEmptyIndex].pid = pid;
	ArrayClock[ClockEmptyIndex].numPagina = pag;
	ArrayClock[ClockEmptyIndex].numSegmento = seg;
	ArrayClock[ClockEmptyIndex++].bitReferencia = 1;

}

void quitarPaginaDeArrayClock(uint32_t pid, uint16_t seg, uint16_t pag) {

	int i;
	for (i = 0; ArrayClock[i].pid != pid || ArrayClock[i].numSegmento != seg || ArrayClock[i].numPagina != pag; ++i)
		;

	/* Muevo todos los registros una posición, pisando el de la página que se fue de memoria */
	for (++i; i < ClockEmptyIndex; ++i) {

		ArrayClock[i-1].pid = ArrayClock[i].pid;
		ArrayClock[i-1].numPagina = ArrayClock[i].numPagina;
		ArrayClock[i-1].numSegmento = ArrayClock[i].numSegmento;
		ArrayClock[i-1].bitReferencia = ArrayClock[i].bitReferencia;

	}

	--ClockEmptyIndex;

}

void imprimirArrayClock(void) {

	int i;

	pthread_mutex_lock(&LogMutex);
	log_trace(Logger, "Algoritmo de Sustitución: Clock:");
	log_trace(Logger, "Posición del puntero: %u", ClockIndex);
	log_trace(Logger, "Lista de Páginas en Memoria:");
	log_trace(Logger, "%-12s%-14s%-14s%-s", "PID", "Nº Segmento", "Nº Página", "Bit de Referencia");
	pthread_mutex_unlock(&LogMutex);

	printf("Algoritmo de Sustitución: Clock\nPosición del puntero: %u\nLista de Páginas en Memoria:\n%-12s%-14s%-14s%-s\n", ClockIndex, "PID", "Nº Segmento", "Nº Página", "Bit de Referencia");

	for (i = 0; i < ClockEmptyIndex; ++i) {
		pthread_mutex_lock(&LogMutex);
		log_trace(Logger, "%-12u%-12u %-12u%-1u", ArrayClock[i].pid, ArrayClock[i].numSegmento, ArrayClock[i].numPagina, ArrayClock[i].bitReferencia);
		pthread_mutex_unlock(&LogMutex);

		printf("%-12u%-12u %-12u%-1u\n", ArrayClock[i].pid, ArrayClock[i].numSegmento, ArrayClock[i].numPagina, ArrayClock[i].bitReferencia);
	}

}
