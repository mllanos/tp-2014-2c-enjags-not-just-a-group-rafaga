/*
 * Clock.c
 *
 *  Created on: 13/10/2014
 *      Author: matias
 */

#include "Clock.h"

void seleccionVictimaClock(t_segmento** tabla,uint32_t *pid,uint16_t *seg,uint16_t *pag) {

	uint32_t inPid = *pid;
	uint16_t inSeg = *seg;
	uint16_t inPag = *pag;

	for(;ArrayClock[ClockIndex = ClockIndex % CantidadMarcosTotal].bitReferencia;++ClockIndex)
		ArrayClock[ClockIndex].bitReferencia = 0;

	t_clock_node* pagMenosUsada = ArrayClock + ClockIndex;	/* Es equivalente a &ArrayClock[ClockIndex] */

	*pid = pagMenosUsada->pid;
	*pag = pagMenosUsada->numPagina;
	*seg = pagMenosUsada->numSegmento;
	*tabla = tablaDelProceso(pagMenosUsada->pid);

	pagMenosUsada->pid = inPid;
	pagMenosUsada->numPagina = inPag;
	pagMenosUsada->numSegmento = inSeg;
	pagMenosUsada->bitReferencia = 1;

}

void actualizarEnArrayClock(uint32_t pid,uint16_t seg,uint16_t pag) {

	int i;
	for(i=0;ArrayClock[i].pid != pid || ArrayClock[i].numSegmento != seg || ArrayClock[i].numPagina != pag;++i)
		;

	ArrayClock[i].bitReferencia = 1;

}

void agregarPaginaEnArrayClock(uint32_t pid,uint16_t seg,uint16_t pag) {

	ArrayClock[ClockEmptyIndex].pid = pid;
	ArrayClock[ClockEmptyIndex].numPagina = pag;
	ArrayClock[ClockEmptyIndex].numSegmento = seg;
	ArrayClock[ClockEmptyIndex++].bitReferencia = 1;

}

void quitarPaginaDeArrayClock(uint32_t pid,uint16_t seg,uint16_t pag) {

	int i;
	for(i=0;ArrayClock[i].pid != pid || ArrayClock[i].numSegmento != seg || ArrayClock[i].numPagina != pag;++i)
		;

	for(++i;i < CantidadMarcosTotal;++i) {

		ArrayClock[i-1].pid = ArrayClock[i].pid;
		ArrayClock[i-1].numPagina = ArrayClock[i].numPagina;
		ArrayClock[i-1].numSegmento = ArrayClock[i].numSegmento;
		ArrayClock[i-1].bitReferencia = ArrayClock[i].bitReferencia;

	}

	--ClockEmptyIndex;

}
