/*
 * administradorDeMemoria.h
 *
 *  Created on: 12/10/2014
 *      Author: matias
 */

#ifndef ADMINISTRADORDEMEMORIA_H_
#define ADMINISTRADORDEMEMORIA_H_

#include <commons/collections/dictionary.h>
#include "msp.h"

/* Funciones Macro */
#define segmentoValido(TABLA,SEG) (TABLA[SEG].limite != 0)
#define excedeLimiteSegmento(TABLA,SEG) (TABLA[SEG].limite != 0)
#define paginaEnMemoria(TABLA,SEG,PAG) TABLA[SEG].tablaPaginas[PAG].bitPresencia
#define generarDireccionLogica(SEG,PAG,OFFSET) ((SEG << 24) + (PAG << 12) + OFFSET)
#define bytesLibresPagina(TABLA,SEG,PAG) (TABLA[SEG].bytesOcupados - PAG * PAG_SIZE)
#define paginaValida(TABLA,SEG,PAG) (PAG < divRoundUp(TABLA[SEG].bytesOcupados,PAG_SIZE))
#define tablaDelProceso(PID) (t_segmento*) dictionary_get(TablaSegmentosGlobal,itoa(PID))
#define direccionFisica(TABLA,SEG,PAG,OFFSET) (MemoriaPrincipal[TABLA[SEG].tablaPaginas[PAG].numMarco].marco + OFFSET)
/* FIN Funciones Macro */

/* Variables Globales */
bit *MarcosLibres;					/* BitVector. Indica de cada marco si está o no ocupado (1: ocupado - 0: disponible)*/
t_marco *MemoriaPrincipal;
uint32_t EspacioDisponible;
uint32_t CantidadMarcosTotal;
uint32_t CantPaginasTotalMax;
uint32_t CantPaginasEnSwapMax;
t_dictionary *TablaSegmentosGlobal;	/* Contiene el puntero a la tabla local de cada proceso. Usa el PID como key. */
/* FIN Variables Globales */

/* Funciones */
uint32_t crearSegmento(uint32_t pid, size_t size);
void destruirSegmento(uint32_t pid, uint16_t numeroSegmento);
char* solicitarMemoria(uint32_t pid,uint32_t direccionLogica,uint32_t size);
t_msg_id escribirMemoria(uint32_t pid,uint32_t direccionLogica,char* bytesAEscribir,uint32_t size);
uint16_t primerEntradaLibre(t_segmento *tabla);
t_segmento* traducirDireccion(uint32_t pid,uint32_t direccionLogica,uint16_t *numSegmento,uint16_t *numPagina,uint8_t *offset);
void traerPaginaAMemoria(void);
/* FIN Funciones */

#endif /* ADMINISTRADORDEMEMORIA_H_ */
