/*
 * LRU.c
 *
 *  Created on: 13/10/2014
 *      Author: matias
 */

#include "LRU.h"

void seleccionVictimaLRU(t_segmento** tabla, uint32_t *pid, uint16_t *seg, uint16_t *pag) {

	char *stringPID;
	uint32_t inPid = *pid;
	uint16_t inSeg = *seg;
	uint16_t inPag = *pag;

	t_LRU_node *pagMenosUsada = (t_LRU_node *) list_remove(ListaLRU, 0);

	*pid = pagMenosUsada->pid;
	*pag = pagMenosUsada->numPagina;
	*seg = pagMenosUsada->numSegmento;
	*tabla = tablaDelProceso(stringPID = string_uitoa(*pid));

	free(stringPID);

	pagMenosUsada->pid = inPid;			/* Aprovecho que el nodo está creado, le cargo los datos de la nueva página, y agrego el nodo al final */
	pagMenosUsada->numPagina = inPag;
	pagMenosUsada->numSegmento = inSeg;

	list_add(ListaLRU, pagMenosUsada);

}

void actualizarEnListaLRU(uint32_t pid, uint16_t seg, uint16_t pag) {

	ListVarPid = pid;
	ListVarSeg = seg;
	ListVarPag = pag;

	list_add(ListaLRU, list_remove_by_condition(ListaLRU, match));

}

void agregarPaginaAListaLRU(uint32_t pid, uint16_t seg, uint16_t pag) {

	t_LRU_node *nodo = malloc(sizeof(t_LRU_node));

	nodo->pid = pid;
	nodo->numPagina = pag;
	nodo->numSegmento = seg;

	list_add(ListaLRU, nodo);

}

void quitarPaginaDeListaLRU(uint32_t pid, uint16_t seg, uint16_t pag) {

	ListVarPid = pid;
	ListVarSeg = seg;
	ListVarPag = pag;

	free(list_remove_by_condition(ListaLRU, match));

}

bool match(void *LRU_node) {

	t_LRU_node *node = (t_LRU_node *) LRU_node;

	return node->pid == ListVarPid && node->numSegmento == ListVarSeg && node->numPagina == ListVarPag;

}

void imprimirListaLRU(void) {

	pthread_mutex_lock(&LogMutex);
	log_trace(Logger, "Algoritmo de Sustitución: LRU");
	log_trace(Logger, "Lista de Páginas en Memoria:");
	log_trace(Logger, "%-12s%-14s%-s", "PID", "Nº Segmento", "Nº Página");
	pthread_mutex_unlock(&LogMutex);

	printf("Algoritmo de Sustitución: LRU\nLista de Páginas en Memoria:\n%-12s%-14s%-s\n","PID","Nº Segmento","Nº Página");

	list_iterate(ListaLRU, imprimirLRU_node);
}

void imprimirLRU_node(void *data) {

	t_LRU_node *nodo = (t_LRU_node *) data;

	pthread_mutex_lock(&LogMutex);
	log_trace(Logger, "%-12u%-12u %-u", nodo->pid, nodo->numSegmento, nodo->numPagina);
	pthread_mutex_unlock(&LogMutex);

	printf("%-12u%-12u %-u\n", nodo->pid, nodo->numSegmento, nodo->numPagina);
}
