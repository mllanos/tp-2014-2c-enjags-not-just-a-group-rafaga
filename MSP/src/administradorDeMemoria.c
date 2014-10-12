/*
 * controladorDeMemoria.c
 *
 *  Created on: 09/10/2014
 *      Author: matias
 */

#include "administradorDeMemoria.h"

char* itoa(int n) {
	return "";
}

void inicializarMSP(void) {

	int i;

	CantidadMarcosTotal = MaxMem / PAG_SIZE;	// Preguntar que pasa si la memoria no se puede repartir en exactamente n marcos
	CantPaginasEnSwapMax = MaxSwap / PAG_SIZE;
	CantPaginasTotalMax = CantidadMarcosTotal + CantPaginasEnSwapMax;
	EspacioDisponible = MaxMem + MaxSwap;

	MemoriaPrincipal = malloc(CantidadMarcosTotal);
	MarcosLibres = malloc(CantidadMarcosTotal);

	for(i=0;i < CantidadMarcosTotal;++i)	/* Inicializo el bit array con 0 (todos los marcos disponibles) */
		MarcosLibres[i] = 0;

	dictionary_create(TablaSegmentosGlobal);

}

uint32_t crearSegmento(uint32_t pid, size_t size) {

	uint16_t numSegmento;

	if(size > SEG_MAX_SIZE)
		return INVALID_SEG_SIZE;

	if(EspacioDisponible >= size) {

		t_segmento *tablaLocal;

		if( ( tablaLocal = tablaDelProceso(pid) ) == NULL )	{				//intentar evitar la conversion a char con itoa; el NULL está por el warning
			dictionary_put(TablaSegmentosGlobal,itoa(pid),tablaLocal = malloc(NUM_SEG_MAX * sizeof(t_segmento)));	/* Creo la tabla local del proceso PID */

			for(numSegmento=0;numSegmento < NUM_SEG_MAX;++numSegmento) {											/* Inicializo la Tabla Local de Segmentos */
				tablaLocal[numSegmento].limite = 0;
				tablaLocal[numSegmento].bytesOcupados = 0;
			}
		}

		if( (numSegmento = primerEntradaLibre(tablaLocal)) == NUM_SEG_MAX )
			return MAX_SEG_NUM_REACHED;

		EspacioDisponible -= size;
		tablaLocal[numSegmento].limite = size;
		tablaLocal[numSegmento].tablaPaginas = malloc(divRoundUp(size,PAG_SIZE) * sizeof(t_pagina));

	}
	else
		return FULL_MEMORY;

	return generarDireccionLogica(numSegmento,0,0);
}

t_msg_id escribirMemoria(uint32_t pid,uint32_t direccionLogica,char* bytesAEscribir,uint32_t size) {

	uint8_t offset;
	uint16_t numSegmento,numPagina;
	t_segmento* tablaLocal;

	if((tablaLocal = traducirDireccion(pid,direccionLogica,&numSegmento,&numPagina,&offset)) != NULL) { //el NULL está por el warning

		size_t bytesEscritos,bytesPorEscribir = size;
		size_t bytesLibresPagina = bytesLibresPagina(tablaLocal,numSegmento,numPagina);

		while(bytesPorEscribir) {

			if(!paginaEnMemoria(tablaLocal,numSegmento,numPagina))
				traerPaginaAMemoria();

			memcpy(direccionFisica(tablaLocal,numSegmento,numPagina,offset),bytesAEscribir,bytesEscritos = min(bytesLibresPagina,bytesPorEscribir));
			if(bytesLibresPagina != PAG_SIZE) bytesLibresPagina = PAG_SIZE;
			tablaLocal[numSegmento].bytesOcupados += bytesEscritos;
			EspacioDisponible -= bytesEscritos;
			bytesPorEscribir -= bytesEscritos;
			++numPagina;

		}
	}
	else
		return -1;//SEGMENTATION_FAULT; (cambiar en utiles)

	return 0;//OK_WRITE; (cambiar en utiles)

}

void destruirSegmento(uint32_t pid, uint16_t numeroSegmento){



}

char* solicitarMemoria(uint32_t pid,uint32_t direccionLogica,uint32_t size) {

	return "";

}

uint16_t primerEntradaLibre(t_segmento *tabla) {

	uint16_t i;

	for(i=0;tabla[i].limite != 0 && i < NUM_SEG_MAX;++i)
		;

	return i;
}

t_segmento* traducirDireccion(uint32_t pid,uint32_t direccionLogica,uint16_t *numSegmento,uint16_t *numPagina,uint8_t *offset) {

	t_segmento* tabla;

	*numSegmento = direccionLogica >> 20;
	*numPagina = (direccionLogica << 12) >> 20;
	*offset = (direccionLogica << 24) >> 24;	//creo que no hace falta, asignándolo a un int de menor tamaño elimina los bits de más

	/* Si el segmento o la página son inválidas asigna NULL a tabla. Si la tabla no existiera la retorna directamente (ya que vale NULL por dictionary_get() */
	if( ( tabla = (t_segmento*) dictionary_get(TablaSegmentosGlobal,itoa(pid)) ) && ( !segmentoValido(tabla,*numSegmento) || !paginaValida(tabla,*numSegmento,*numPagina) ) )
		tabla = NULL;

	return tabla;	/* Retorna o bien el puntero a la tabla de segmentos del proceso, o bien NULL si la dirección es inválida */

}

void traerPaginaAMemoria(void) {



}
