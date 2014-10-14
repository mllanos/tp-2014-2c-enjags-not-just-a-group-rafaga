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
		RutinaSeleccionPaginaVictima = seleccionVictimaClock;
		AgregarPaginaAEstructuraSustitucion = agregarPaginaEnArrayClock;
		QuitarDeEstructuraDeSeleccion = quitarPaginaDeArrayClock;
		ActualizarEnEstructuraDeSustitucion = actualizarEnArrayClock;
	}
	else {
		ListaLRU = list_create();
		RutinaSeleccionPaginaVictima = seleccionVictimaLRU;
		AgregarPaginaAEstructuraSustitucion = agregarPaginaAListaLRU;
		QuitarDeEstructuraDeSeleccion = quitarPaginaDeListaLRU;
		ActualizarEnEstructuraDeSustitucion = actualizarEnListaLRU;
	}

}

uint32_t crearSegmento(uint32_t pid, size_t size, t_msg_id* id) {

	uint16_t numSegmento;
	uint16_t cantPaginas = divRoundUp(size,PAG_SIZE);

	if(size > SEG_MAX_SIZE) {
		*id = INVALID_SEG_SIZE;
		return 0;
	}

	if(CantPaginasDisponibles >= cantPaginas) {

		int pag;
		char* stringPID;
		char* stringSEG;
		t_segmento *tablaLocal;

		if( ( tablaLocal = tablaDelProceso(pid) ) == NULL )	{				//intentar evitar la conversion a char con itoa; el NULL está por el warning
			dictionary_put(TablaSegmentosGlobal,string_itoa(pid),tablaLocal = malloc(NUM_SEG_MAX * sizeof(t_segmento)));	/* Creo la tabla local del proceso PID */

			for(numSegmento=0;numSegmento < NUM_SEG_MAX;++numSegmento)													/* Inicializo la Tabla Local de Segmentos */
				tablaLocal[numSegmento].limite = 0;

		}

		if( (numSegmento = primerEntradaLibre(tablaLocal)) == NUM_SEG_MAX ) {
			*id = MAX_SEG_NUM_REACHED;
			return 0;
		}

		CantPaginasDisponibles -= cantPaginas;
		tablaLocal[numSegmento].limite = size;
		tablaLocal[numSegmento].bytesOcupados = 0;
		tablaLocal[numSegmento].tablaPaginas = malloc(divRoundUp(size,PAG_SIZE) * sizeof(t_pagina));

		stringPID = string_itoa(pid);
		stringSEG = string_itoa(numSegmento);

		/* Creo las páginas que va a ocupar el segmento en el espacio de SWAP */
		for(pag = 0;pag < cantPaginas;++pag)
			create_file(string_from_format("%s%s%s%s",SwapPath,stringPID,stringSEG,string_itoa(pag)),PAG_SIZE);

		free(stringPID);
		free(stringSEG);
	}
	else {
		*id = FULL_MEMORY;
		return 0;
	}

	*id = OK_CREATE;
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

			ActualizarEnEstructuraDeSustitucion(pid,numSegmento,numPagina);
			memcpy(direccionFisica(tablaLocal,numSegmento,numPagina,offset),bytesAEscribir+i,bytesEscritos = min((PAG_SIZE - offset),bytesPorEscribir));
			tablaLocal[numSegmento].bytesOcupados += bytesEscritos;
			bytesPorEscribir -= bytesEscritos;
			i += bytesEscritos;
			++numPagina;
			offset = 0;
		}
		//no olvidarse de liberar bytesAEscribir, o acá adentro, o en la rutina que atiende a los procesos
	}
	else
		return SEGMENTATION_FAULT;

	return OK_WRITE;

}

char* solicitarMemoria(uint32_t pid,uint32_t direccionLogica,uint32_t size,t_msg_id id) {

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

			ActualizarEnEstructuraDeSustitucion(pid,numSegmento,numPagina);
			memcpy(bytesSolicitados+i,direccionFisica(tablaLocal,numSegmento,numPagina,offset),bytesLeidos = min((PAG_SIZE - offset),bytesPorLeer));
			bytesPorLeer -= bytesLeidos;
			i += bytesLeidos;
			++numPagina;
			offset = 0;

		}
	}
	else {
		*id = SEGMENTATION_FAULT;
		return NULL;
	}

	*id = OK_REQUEST;
	return bytesSolicitados;

}

t_msg_id destruirSegmento(uint32_t pid, uint16_t numeroSegmento){

	t_segmento* tabla = tablaDelProceso(pid);

	if(tabla && segmentoValido(tabla,numeroSegmento)) {

		int pag;
		char* shellInstruction;
		char* stringPID;
		char* stringSEG;
		uint16_t cantPaginas = cantidadPaginas(tabla,numeroSegmento);

		for(pag = 0;pag < cantPaginas;++pag)
			if(paginaEnMemoria(tabla,numeroSegmento,pag)) {
				liberarMarco(tabla,numeroSegmento,pag);
				QuitarDeEstructuraDeSeleccion(pid,numeroSegmento,pag);
			}

		/* Borra todas las páginas swappeadas del segmento */

		stringPID = string_itoa(pid);
		stringSEG = string_itoa(numeroSegmento);
		shellInstruction = string_from_format("%s%s%s%s%s%s%s","cd ",SwapPath,"\n","rm ",stringPID,stringSEG,"*");

		system(shellInstruction);

		free(tabla[numeroSegmento].tablaPaginas);
		tabla[numeroSegmento].limite = 0;

		free(shellInstruction);
		free(stringPID);
		free(stringSEG);
		//validar si es el único segmento del proceso, en ese caso probablemente habría que eliminar la entrada del diccionario
	}
	else
		return INVALID_DIR;

	return OK_DESTROY;

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

	uint32_t numMarco;
	char* data;
	char* path = obtenerSwapPath(pid,seg,pag);

	if((numMarco = marcoVacio()) == CantidadMarcosTotal)
		numMarco = rutinaSustitucion(pid,seg,pag);
	else
		AgregarPaginaAEstructuraSustitucion(pid,seg,pag);

	memcpy(MemoriaPrincipal[numMarco].marco,data = read_file(path,PAG_SIZE),PAG_SIZE);
	remove(path);

	paginaEnMemoria(tabla,seg,pag) = true;
	MemoriaPrincipal[numMarco].ocupado = true;

	free(data);
	free(path);

}

uint32_t marcoVacio(void) {

	uint32_t i;

	for(i=0;MemoriaPrincipal[i].ocupado && i < CantidadMarcosTotal;++i)
		;

	return i;
}

uint32_t rutinaSustitucion(uint32_t inPid,uint16_t seg,uint16_t pag) {

	char* path;
	uint32_t outPid = inPid;
	t_segmento* tabla;
	uint16_t numPagina = pag;
	uint32_t numMarcoLiberado;
	uint16_t numSegmento = seg;

	RutinaSeleccionPaginaVictima(&tabla,&outPid,&numSegmento,&numPagina);

	numMarcoLiberado = tabla[numSegmento].tablaPaginas[numPagina].numMarco;

	path = obtenerSwapPath(outPid,numSegmento,numPagina);

	write_file(path,direccionFisica(tabla,numSegmento,numPagina,0),PAG_SIZE);

	paginaEnMemoria(tabla,numSegmento,numPagina) = false;

	MemoriaPrincipal[numMarcoLiberado].ocupado = false;

	free(path);

	return numMarcoLiberado;

}
