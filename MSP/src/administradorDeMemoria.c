/*
 * controladorDeMemoria.c
 *
 *  Created on: 09/10/2014
 *      Author: matias
 */

#include "administradorDeMemoria.h"

void inicializarMSP(char* swapPath) {

	int i;

	SwapPath = swapPath;

	CantidadMarcosTotal = MaxMem / PAG_SIZE;	// Preguntar que pasa si la memoria no se puede repartir en exactamente n marcos
	CantPaginasEnSwapMax = MaxSwap / PAG_SIZE;
	CantPaginasDisponibles = CantPaginasTotalMax = CantidadMarcosTotal + CantPaginasEnSwapMax;

	MemoriaPrincipal = malloc(CantidadMarcosTotal);

	for(i=0;i < CantidadMarcosTotal;++i)	/* Inicializo el bit ocupado con 0 (todos los marcos disponibles) */
		MemoriaPrincipal[i].ocupado = 0;

	dictionary_create(TablaSegmentosGlobal);

	if(strcmp(AlgoritmoSustitucion,"LRU")) {	/* Elige Clock por default */
		ArrayClock = malloc(CantidadMarcosTotal * sizeof(t_clock_node));
		rutinaSustitucion = rutinaClock;
	}
	else {
		ListaLRU = list_create();
		rutinaSustitucion = rutinaLRU;
	}

}

uint32_t crearSegmento(uint32_t pid, size_t size) {

	uint16_t numSegmento;
	uint16_t cantPaginas = divRoundUp(size,PAG_SIZE);

	if(size > SEG_MAX_SIZE)
		return INVALID_SEG_SIZE;

	if(CantPaginasDisponibles >= cantPaginas) {

		t_segmento *tablaLocal;

		if( ( tablaLocal = tablaDelProceso(pid) ) == NULL )	{				//intentar evitar la conversion a char con itoa; el NULL está por el warning
			dictionary_put(TablaSegmentosGlobal,string_itoa(pid),tablaLocal = malloc(NUM_SEG_MAX * sizeof(t_segmento)));	/* Creo la tabla local del proceso PID */

			for(numSegmento=0;numSegmento < NUM_SEG_MAX;++numSegmento) {											/* Inicializo la Tabla Local de Segmentos */
				tablaLocal[numSegmento].limite = 0;
				tablaLocal[numSegmento].bytesOcupados = 0;
			}
		}

		if( (numSegmento = primerEntradaLibre(tablaLocal)) == NUM_SEG_MAX )
			return MAX_SEG_NUM_REACHED;

		CantPaginasDisponibles -= cantPaginas;
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

	if((tablaLocal = traducirDireccion(pid,direccionLogica,&numSegmento,&numPagina,&offset)) && bytesLibresSegmento(tablaLocal,numSegmento) >= size) {
	//Si excede el tamaño se escribe lo que puede o también es SEGMENTATION_FAULT de una?
		int i = 0;
		size_t bytesEscritos,bytesPorEscribir = size;

		while(bytesPorEscribir) {

			if(!paginaEnMemoria(tablaLocal,numSegmento,numPagina))
				traerPaginaAMemoria(tablaLocal,pid,numSegmento,numPagina);

			memcpy(direccionFisica(tablaLocal,numSegmento,numPagina,offset),bytesAEscribir+i,bytesEscritos = min((PAG_SIZE - offset),bytesPorEscribir));
			tablaLocal[numSegmento].bytesOcupados += bytesEscritos;
			bytesPorEscribir -= bytesEscritos;
			i += bytesEscritos;
			++numPagina;
			offset = 0;
		}
	}
	else
		return -1;//SEGMENTATION_FAULT;

	return 0;//OK_WRITE;

}

char* solicitarMemoria(uint32_t pid,uint32_t direccionLogica,uint32_t size) {

	uint8_t offset;
	char* bytesSolicitados;
	t_segmento* tablaLocal;
	uint16_t numSegmento,numPagina;

	if((tablaLocal = traducirDireccion(pid,direccionLogica,&numSegmento,&numPagina,&offset)) && bytesOcupadosDesde(tablaLocal,numSegmento,numPagina,offset) >= size) {

		int i = 0;
		size_t bytesLeidos,bytesPorLeer = size;
		bytesSolicitados = malloc(size);

		while(bytesPorLeer) {

			if(!paginaEnMemoria(tablaLocal,numSegmento,numPagina))
				traerPaginaAMemoria(tablaLocal,pid,numSegmento,numPagina);
			memcpy(bytesSolicitados,direccionFisica(tablaLocal,numSegmento,numPagina,offset),bytesLeidos = min((PAG_SIZE - offset),bytesPorLeer));
			bytesPorLeer -= bytesLeidos;
			i += bytesLeidos;
			++numPagina;
			offset = 0;

		}
	}
	else
		return "SEGMENTATION_FAULT";

	return bytesSolicitados;//OK_WRITE;

}

void destruirSegmento(uint32_t pid, uint16_t numeroSegmento){



}

uint16_t primerEntradaLibre(t_segmento *tabla) {

	uint16_t i;

	for(i=0;tabla[i].limite != 0 && i < NUM_SEG_MAX;++i)
		;

	return i;
}

t_segmento* traducirDireccion(uint32_t pid,uint32_t direccionLogica,uint16_t *numSegmento,uint16_t *numPagina,uint8_t *offset) {

	t_segmento* tabla;

	decodeDirLogica(direccionLogica,numSegmento,numPagina,offset);

	/* Si el segmento o la página son inválidas asigna NULL a tabla. Si la tabla no existiera la retorna directamente (ya que vale NULL por dictionary_get() */
	if( ( tabla = (t_segmento*) dictionary_get(TablaSegmentosGlobal,string_itoa(pid)) ) && ( !segmentoValido(tabla,*numSegmento) || !paginaValida(tabla,*numSegmento,*numPagina,*offset) ) )
		tabla = NULL;

	return tabla;	/* Retorna o bien el puntero a la tabla de segmentos del proceso, o bien NULL si la dirección es inválida */

}

void traerPaginaAMemoria(t_segmento *tabla,uint32_t pid,uint16_t seg,uint16_t pag) {

	int i;
	char* path = obtenerSwapPath(pid,seg,pag);

	if((i = marcoVacio()) == CantidadMarcosTotal)
		i = rutinaSustitucion();

	memcpy(MemoriaPrincipal[i].marco,read_file(path,PAG_SIZE),PAG_SIZE);
	remove(path);
	paginaEnMemoria(tabla,seg,pag) = true;
	MemoriaPrincipal[i].ocupado = true;

}

uint16_t marcoVacio(void) {

	uint16_t i;

	for(i=0;MemoriaPrincipal[i].ocupado && i < CantidadMarcosTotal;++i)
		;

	return i;
}

uint32_t rutinaLRU(void) {

	uint16_t numSegmento,numPagina;

	t_LRU_node* pagMenosUsada = (t_LRU_node*) list_remove(ListaLRU,0);

	t_segmento* tabla = tablaDelProceso(pagMenosUsada->pid);

	numSegmento = pagMenosUsada->numSegmento;
	numPagina = pagMenosUsada->numPagina;

	uint32_t numMarcoLiberado = tabla[numSegmento].tablaPaginas[numPagina].numMarco;

	char* path = obtenerSwapPath(pagMenosUsada->pid,numSegmento,numPagina);

	write_file(path,direccionFisica(tabla,numSegmento,numPagina,0),PAG_SIZE);

	paginaEnMemoria(tabla,numSegmento,numPagina) = false;

	MemoriaPrincipal[numMarcoLiberado].ocupado = false;

	return numMarcoLiberado;

}

uint32_t rutinaClock(void) {

	int i;
	uint16_t numSegmento,numPagina;
	t_clock_node *pagMenosUsada;

	for(i=0;ArrayClock[i%CantidadMarcosTotal].bitReferencia;++i)
		ArrayClock[i].bitReferencia = 0;

	pagMenosUsada = ArrayClock + (i%CantidadMarcosTotal);

	t_segmento* tabla = tablaDelProceso(pagMenosUsada->pid);

	numSegmento = pagMenosUsada->numSegmento;
	numPagina = pagMenosUsada->numPagina;

	uint32_t numMarcoLiberado = tabla[numSegmento].tablaPaginas[numPagina].numMarco;

	char* path = obtenerSwapPath(pagMenosUsada->pid,numSegmento,numPagina);

	write_file(path,direccionFisica(tabla,numSegmento,numPagina,0),PAG_SIZE);

	paginaEnMemoria(tabla,numSegmento,numPagina) = false;

	MemoriaPrincipal[numMarcoLiberado].ocupado = false;

	return numMarcoLiberado;

}


