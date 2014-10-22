/*
 * LRU.h
 *
 *  Created on: 13/10/2014
 *      Author: matias
 */

#ifndef LRU_H_
#define LRU_H_

#include <commons/collections/list.h>
#include "administradorDeMemoria.h"

typedef struct {
	uint32_t pid;
	uint16_t numSegmento;
	uint16_t numPagina;
} __attribute__ ((__packed__)) t_LRU_node;

/* Variables Globales */
t_list *ListaLRU;					/* Estructura de control para el algoritmo LRU */
uint32_t ListVarPid;
uint16_t ListVarSeg;
uint16_t ListVarPag;
/* FIN Variables Globales */

/* Funciones */
bool match(void* LRU_node);
void imprimirListaLRU(void);
void imprimirLRU_node(void *data);
void actualizarEnListaLRU(uint32_t pid,uint16_t seg,uint16_t pag);
void agregarPaginaAListaLRU(uint32_t pid,uint16_t seg,uint16_t pag);
void quitarPaginaDeListaLRU(uint32_t pid,uint16_t seg,uint16_t pag);
void seleccionVictimaLRU(t_segmento** tabla,uint32_t *pid,uint16_t *seg,uint16_t *pag);
/* FIN Funciones */

#endif /* LRU_H_ */
