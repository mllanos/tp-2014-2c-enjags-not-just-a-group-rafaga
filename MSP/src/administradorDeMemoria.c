/*
 * controladorDeMemoria.c
 *
 *  Created on: 09/10/2014
 *      Author: matias
 */

#include "administradorDeMemoria.h"

void inicializarMSP(void) {	//por ahí podría borrar acá lo que hay en la carpeta swap ya que estoy

	int i;

	CantPaginasEnSwapMax = MaxSwap / PAG_SIZE;
	CantPaginasEnSwapDisponibles = CantPaginasEnSwapMax - 1; 					/* Dejo un lugar disponible para hacer los intercambios */
	CantPaginasEnMemoriaDisponibles = CantidadMarcosTotal = MaxMem / PAG_SIZE;
	CantPaginasDisponibles = CantPaginasEnMemoriaDisponibles + CantPaginasEnSwapDisponibles;


	if ((MemoriaPrincipal = malloc(CantidadMarcosTotal * sizeof(t_marco))) == NULL)
		perror("MALLOC");

	/* Inicializo el bit ocupado con 0 (todos los marcos disponibles) */
	for (i = 0; i < CantidadMarcosTotal; ++i)
		MemoriaPrincipal[i].ocupado = 0;

	TablaSegmentosGlobal = dictionary_create();

	/* Configuro las rutinas del algoritmo de selección a utilizar. Elige Clock por default */
	if (strcmp(AlgoritmoSustitucion,"LRU")) {
		ArrayClock = malloc(CantidadMarcosTotal * sizeof(t_clock_node));

		RutinaSeleccionPaginaVictima = seleccionVictimaClock;
		ImprimirInfoAlgoritmosSustitucion = imprimirArrayClock;
		QuitarDeEstructuraDeSeleccion = quitarPaginaDeArrayClock;
		ActualizarEnEstructuraDeSustitucion = actualizarEnArrayClock;
		AgregarPaginaAEstructuraSustitucion = agregarPaginaEnArrayClock;
	}
	else {
		ListaLRU = list_create();

		RutinaSeleccionPaginaVictima = seleccionVictimaLRU;
		ImprimirInfoAlgoritmosSustitucion = imprimirListaLRU;
		QuitarDeEstructuraDeSeleccion = quitarPaginaDeListaLRU;
		ActualizarEnEstructuraDeSustitucion = actualizarEnListaLRU;
		AgregarPaginaAEstructuraSustitucion = agregarPaginaAListaLRU;
	}

	pthread_mutex_init(&LogMutex, NULL);
	pthread_mutex_init(&MemMutex, NULL);

}

uint32_t crearSegmento(uint32_t pid, size_t size, t_msg_id* id) {

	uint16_t numSegmento;
	uint16_t cantPaginas = divRoundUp(size,PAG_SIZE);

	if (size > SEG_MAX_SIZE) {
		*id = INVALID_SEG_SIZE;
		return 0;
	}

	if (CantPaginasDisponibles >= cantPaginas) {

		int pag;
		t_segmento *tablaLocal;
		char *stringPID = string_uitoa(pid);

		if ((tablaLocal = tablaDelProceso(stringPID)) == NULL)	{
			dictionary_put(TablaSegmentosGlobal, stringPID, tablaLocal = malloc(NUM_SEG_MAX * sizeof(t_segmento)));	/* Creo la tabla local del proceso PID */

			for (numSegmento = 0; numSegmento < NUM_SEG_MAX; ++numSegmento)													/* Inicializo la Tabla Local de Segmentos */
				tablaLocal[numSegmento].limite = 0;
		}

		if ((numSegmento = primerEntradaLibre(tablaLocal)) == NUM_SEG_MAX) {
			*id = MAX_SEG_NUM_REACHED;
			return 0;
		}

		CantPaginasDisponibles -= cantPaginas;
		tablaLocal[numSegmento].limite = size;
		tablaLocal[numSegmento].bytesOcupados = 0;
		tablaLocal[numSegmento].tablaPaginas = malloc(cantPaginas * sizeof(t_pagina));

		/* Creo las páginas que va a ocupar el segmento en el espacio de SWAP */
		for (pag = 0; pag < cantPaginas && CantPaginasEnSwapDisponibles; ++pag, --CantPaginasEnSwapDisponibles) {

			char* path = string_from_format("%s%s-%u-%u", SWAP_PATH, stringPID, numSegmento, pag);

			create_file(path, PAG_SIZE);
			paginaEnMemoria(tablaLocal, numSegmento, pag) = false;

			free(path);
		}

		if (CantPaginasEnSwapDisponibles == 0) {
			pthread_mutex_lock(&LogMutex);
			log_trace(Logger, "Espacio de intercambio llleno.");
			pthread_mutex_unlock(&LogMutex);
		}

		/* Si la SWAP se llena reservo marcos de Memoria Principal para el resto de las páginas */
		for (; pag < cantPaginas; ++pag, --CantPaginasEnMemoriaDisponibles) {

			MemoriaPrincipal[marcoDePagina(tablaLocal, numSegmento, pag) = marcoVacio()].ocupado = true;
			MemoriaPrincipal[marcoDePagina(tablaLocal, numSegmento, pag)].pid = pid;
			paginaEnMemoria(tablaLocal, numSegmento, pag) = true;
			AgregarPaginaAEstructuraSustitucion(pid, numSegmento, pag);

			pthread_mutex_lock(&LogMutex);
			log_trace(Logger, "Marco %u asignado al proceso %s.", marcoDePagina(tablaLocal, numSegmento, pag), stringPID);
			pthread_mutex_unlock(&LogMutex);
		}

		free(stringPID);

		if (CantPaginasEnMemoriaDisponibles == 0) {
			pthread_mutex_lock(&LogMutex);
			log_trace(Logger, "Memoria Principal lllena.");
			pthread_mutex_unlock(&LogMutex);
		}

	}
	else {
		*id = FULL_MEMORY;
		return 0;
	}

	*id = OK_CREATE;
	return generarDireccionLogica(numSegmento,0,0);
}

t_msg_id escribirMemoria(uint32_t pid, uint32_t direccionLogica, char *bytesAEscribir, uint32_t size) {

	uint8_t offset;
	uint16_t numSegmento, numPagina;
	t_segmento* tablaLocal;

	/* Valido la dirección y me fijo que no se sobrepase el límite del segmento. */
	if ((tablaLocal = traducirDireccion(pid, direccionLogica, &numSegmento, &numPagina, &offset)) && bytesEscribiblesDesde(tablaLocal, numSegmento, numPagina, offset) >= size) {

		int i = 0;
		size_t bytesEscritos,bytesPorEscribir = size;

		while (bytesPorEscribir) {

			if (paginaEnMemoria(tablaLocal, numSegmento, numPagina))
				ActualizarEnEstructuraDeSustitucion(pid, numSegmento, numPagina);
			else
				traerPaginaAMemoria(tablaLocal, pid, numSegmento, numPagina);

			memcpy(direccionFisica(tablaLocal,numSegmento,numPagina,offset),bytesAEscribir + i, bytesEscritos = min((PAG_SIZE - offset),bytesPorEscribir));
			tablaLocal[numSegmento].bytesOcupados += bytesEscritos;
			bytesPorEscribir -= bytesEscritos;
			i += bytesEscritos;
			++numPagina;
			offset = 0;
		}

	}
	else
		return SEGMENTATION_FAULT;

	return OK_WRITE;

}

char* solicitarMemoria(uint32_t pid, uint32_t direccionLogica, uint32_t size, t_msg_id* id) {

	uint8_t offset;
	char *bytesSolicitados;
	t_segmento *tablaLocal;
	uint16_t numSegmento, numPagina;

	/* Valido la dirección y me fijo que no se sobrepase el límite del segmento. */
	if ((tablaLocal = traducirDireccion(pid, direccionLogica, &numSegmento, &numPagina, &offset)) && bytesLeiblesDesde(tablaLocal, numSegmento, numPagina, offset) >= size) {

		int i = 0;
		size_t bytesLeidos,bytesPorLeer = size;
		bytesSolicitados = malloc(size);

		while (bytesPorLeer) {

			if (!paginaEnMemoria(tablaLocal,numSegmento,numPagina))
				traerPaginaAMemoria(tablaLocal,pid,numSegmento,numPagina);

			ActualizarEnEstructuraDeSustitucion(pid, numSegmento, numPagina);
			memcpy(bytesSolicitados + i, direccionFisica(tablaLocal, numSegmento, numPagina, offset), bytesLeidos = min((PAG_SIZE - offset), bytesPorLeer));
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

t_msg_id destruirSegmento(uint32_t pid, uint32_t baseSegmento) {

	char *stringPID = string_uitoa(pid);
	t_segmento* tabla = tablaDelProceso(stringPID);
	uint16_t numeroSegmento = segmento(baseSegmento);

	if (tabla && segmentoValido(tabla, numeroSegmento)) {

		char *shellInstruction;
		int pag,cantPagEnMemoria;
		uint16_t cantPaginas = cantidadPaginasTotalDelSegmento(tabla, numeroSegmento);

		for (cantPagEnMemoria = 0, pag = 0; pag < cantPaginas; ++pag)
			if (paginaEnMemoria(tabla, numeroSegmento, pag)) {
				++cantPagEnMemoria;
				++CantPaginasEnMemoriaDisponibles;
				liberarMarco(tabla, numeroSegmento, pag);
				QuitarDeEstructuraDeSeleccion(pid, numeroSegmento, pag);
				pthread_mutex_lock(&LogMutex);
				log_trace(Logger, "Marco %u desasignado al proceso %u.", marcoDePagina(tabla, numeroSegmento, pag), pid);
				pthread_mutex_unlock(&LogMutex);
			}

		/* Borra todas las páginas swappeadas del segmento */
		if (cantPagEnMemoria < cantPaginas) {
			shellInstruction = string_from_format("cd %s\nrm %s-%u-*", SWAP_PATH, stringPID, numeroSegmento);
			system(shellInstruction);
			free(shellInstruction);
		}

		CantPaginasDisponibles += cantPaginas;
		CantPaginasEnSwapDisponibles += cantPaginas - cantPagEnMemoria;
		free(tabla[numeroSegmento].tablaPaginas);
		tabla[numeroSegmento].limite = 0;

		free(stringPID);
		//validar si es el único segmento del proceso, en ese caso probablemente habría que eliminar la entrada del diccionario
		//Por ahi el kernel me puede avisar, total los últimos segmentos en borrarse son stack y código, y coinciden con la finalización del programa
	}
	else
		return INVALID_DIR;

	return OK_DESTROY;

}

uint16_t primerEntradaLibre(t_segmento *tabla) {

	uint16_t i;

	for (i = 0; tabla[i].limite != 0 && i < NUM_SEG_MAX; ++i)
		;

	return i;
}

t_segmento *traducirDireccion(uint32_t pid, uint32_t direccionLogica, uint16_t *numSegmento, uint16_t *numPagina, uint8_t *offset) {

	char *stringPID;
	t_segmento *tabla;

	decodeDirLogica(direccionLogica, numSegmento, numPagina, offset);

	/* Si el segmento o la página son inválidas asigna NULL a tabla. Si la tabla no existiera la retorna directamente (ya que vale NULL por dictionary_get() */
	if ((tabla = (t_segmento*) dictionary_get(TablaSegmentosGlobal, stringPID = string_uitoa(pid))) && (!segmentoValido(tabla,*numSegmento) || !paginaValida(tabla,*numSegmento,*numPagina,*offset)))
		tabla = NULL;

	free(stringPID);

	return tabla;	/* Retorna o bien el puntero a la tabla de segmentos del proceso, o bien NULL si la dirección es inválida */

}

void traerPaginaAMemoria(t_segmento *tabla, uint32_t pid, uint16_t seg, uint16_t pag) {

	uint32_t numMarco;
	char *path = obtenerSwapPath(pid, seg, pag);

	if ((numMarco = marcoVacio()) == CantidadMarcosTotal)
		numMarco = rutinaSustitucion(pid, seg, pag);
	else {
		++CantPaginasEnSwapDisponibles;
		if (--CantPaginasEnMemoriaDisponibles == 0) {
			pthread_mutex_lock(&LogMutex);
			log_trace(Logger, "Memoria Principal lllena.");
			pthread_mutex_unlock(&LogMutex);
		}
		AgregarPaginaAEstructuraSustitucion(pid, seg, pag);
	}

	memcpy_from_file(MemoriaPrincipal[numMarco].marco, path, PAG_SIZE);
	remove(path);
	free(path);

	pthread_mutex_lock(&LogMutex);
	log_trace(Logger, "Intercambio de página %u del segmento %u del proceso %u desde disco.", pag, seg, pid);
	log_trace(Logger, "Marco %u asignado al proceso %u.", numMarco, pid);
	pthread_mutex_unlock(&LogMutex);

	marcoDePagina(tabla, seg, pag) = numMarco;
	paginaEnMemoria(tabla, seg, pag) = true;
	MemoriaPrincipal[numMarco].pid = pid;
	MemoriaPrincipal[numMarco].ocupado = true;

}

uint32_t marcoVacio(void) {

	uint32_t i;

	for (i = 0; MemoriaPrincipal[i].ocupado && i < CantidadMarcosTotal; ++i)
		;

	return i;
}

uint32_t rutinaSustitucion(uint32_t inPid, uint16_t inSeg, uint16_t inPag) {

	char *path;
	t_segmento* outTabla;
	uint32_t outPid = inPid;
	uint16_t outSeg = inSeg;
	uint16_t outPag = inPag;
	uint32_t numMarcoLiberado;

	RutinaSeleccionPaginaVictima(&outTabla, &outPid, &outSeg, &outPag);

	numMarcoLiberado = marcoDePagina(outTabla, outSeg, outPag);

	path = obtenerSwapPath(outPid, outSeg, outPag);

	write_file(path, direccionFisica(outTabla, outSeg, outPag, 0), PAG_SIZE);

	paginaEnMemoria(outTabla, outSeg, outPag) = false;

	MemoriaPrincipal[numMarcoLiberado].ocupado = false;

	pthread_mutex_lock(&LogMutex);
	log_trace(Logger, "Intercambio de página %u del segmento %u del proceso %u hacia disco.", outPag, outSeg, outPid);
	log_trace(Logger, "Marco %u desasignado al proceso %u.", numMarcoLiberado, outPid);
	pthread_mutex_unlock(&LogMutex);

	free(path);

	return numMarcoLiberado;
}

char *string_uitoa(uint32_t number) {
    return string_from_format("%u", number);
}
