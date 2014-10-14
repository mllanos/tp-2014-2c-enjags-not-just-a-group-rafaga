/*
 * LRU.c
 *
 *  Created on: 13/10/2014
 *      Author: matias
 */

#include "LRU.h"

void seleccionVictimaLRU(t_segmento** tabla,uint32_t *pid,uint16_t *seg,uint16_t *pag) {

	uint32_t inPid = *pid;
	uint16_t inSeg = *seg;
	uint16_t inPag = *pag;

	t_LRU_node* pagMenosUsada = (t_LRU_node*) list_remove(ListaLRU,0);

	*pid = pagMenosUsada->pid;
	*pag = pagMenosUsada->numPagina;
	*seg = pagMenosUsada->numSegmento;
	*tabla = tablaDelProceso(pagMenosUsada->pid);

	pagMenosUsada->pid = inPid;
	pagMenosUsada->numPagina = inPag;
	pagMenosUsada->numSegmento = inSeg;

	list_add(ListaLRU,pagMenosUsada);

}

void actualizarEnListaLRU(uint32_t pid,uint16_t seg,uint16_t pag) {

	ListVarPid = pid;
	ListVarSeg = seg;
	ListVarPag = pag;

	list_add(ListaLRU,list_remove_by_condition(ListaLRU,match));

}

void agregarPaginaAListaLRU(uint32_t pid,uint16_t seg,uint16_t pag) {

	t_LRU_node* nodo = malloc(sizeof(t_LRU_node));

	nodo->pid = pid;
	nodo->numPagina = pag;
	nodo->numSegmento = seg;

	list_add(ListaLRU,nodo);

}

void quitarPaginaDeListaLRU(uint32_t pid,uint16_t seg,uint16_t pag) {

	ListVarPid = pid;
	ListVarSeg = seg;
	ListVarPag = pag;

	free(list_remove_by_condition(ListaLRU,match));

}

bool match(void* LRU_node) {

	t_LRU_node* node = (t_LRU_node*) LRU_node;

	return node->pid == ListVarPid && node->numPagina == ListVarPag && node->numSegmento == ListVarSeg;

}
