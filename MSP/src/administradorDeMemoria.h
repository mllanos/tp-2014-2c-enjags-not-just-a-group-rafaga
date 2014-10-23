/*
 * administradorDeMemoria.h
 *
 *  Created on: 12/10/2014
 *      Author: matias
 */

#ifndef ADMINISTRADORDEMEMORIA_H_
#define ADMINISTRADORDEMEMORIA_H_

#include "msp.h"
#include "LRU.h"
#include "Clock.h"
#include <commons/string.h>
#include <commons/collections/dictionary.h>

/* Funciones Macro */

/* Direcciones */
#define decodeDirLogicaSinOffset(DIRLOG,SEG,PAG)\
	*SEG = DIRLOG >> 20;\
	*PAG = (DIRLOG << 12) >> 20;

#define decodeDirLogica(DIRLOG,SEG,PAG,OFFSET)\
	*SEG = DIRLOG >> 20;\
	*PAG = (DIRLOG << 12) >> 20;\
	*OFFSET = (DIRLOG << 24) >> 24; //creo que no hace falta, asignándolo a un int de menor tamaño elimina los bits de más

#define generarDireccionLogica(SEG,PAG,OFFSET) ((SEG << 20) + (PAG << 8) + OFFSET)
#define obtenerSwapPath(PID,SEG,PAG) string_from_format("%s%u-%u-%u",SwapPath,PID,SEG,PAG)
#define tablaDelProceso(PID) (t_segmento*) dictionary_get(TablaSegmentosGlobal,PID)
#define liberarMarco(TABLA,SEG,PAG) MemoriaPrincipal[TABLA[SEG].tablaPaginas[PAG].numMarco].ocupado = 0
#define direccionFisica(TABLA,SEG,PAG,OFFSET) (MemoriaPrincipal[TABLA[SEG].tablaPaginas[PAG].numMarco].marco + OFFSET)

/* Segmentos */
#define segmento(DIRLOG) (DIRLOG >> 20)
#define segmentoValido(TABLA,SEG) (TABLA[SEG].limite != 0)

/* Páginas */
#define marcoDePagina(TABLA,SEG,PAG) TABLA[SEG].tablaPaginas[PAG].numMarco
#define paginaEnMemoria(TABLA,SEG,PAG) TABLA[SEG].tablaPaginas[PAG].bitPresencia
#define cantidadPaginasTotalDelSegmento(TABLA,SEG) divRoundUp(TABLA[SEG].limite,PAG_SIZE)
#define paginaValida(TABLA,SEG,PAG,OFFSET) (bytesOcupadosHasta(PAG,OFFSET) <= TABLA[SEG].limite)

/* Otros */
#define bytesOcupadosHasta(PAG,OFFSET) (PAG * PAG_SIZE + OFFSET)
#define bytesLeiblesDesde(TABLA,SEG,PAG,OFFSET) bytesEscribiblesDesde(TABLA,SEG,PAG,OFFSET)
#define bytesEscribiblesDesde(TABLA,SEG,PAG,OFFSET) (TABLA[SEG].limite - bytesOcupadosHasta(PAG,OFFSET))

/* FIN Funciones Macro */


/* Variables Globales */
t_marco *MemoriaPrincipal;
uint32_t CantidadMarcosTotal;
uint32_t CantPaginasTotalMax;
uint32_t CantPaginasEnSwapMax;
uint32_t CantPaginasDisponibles;
uint32_t CantPaginasEnSwapDisponibles;
uint32_t CantPaginasEnMemoriaDisponibles;
t_dictionary *TablaSegmentosGlobal;			/* Contiene el puntero a la tabla local de cada proceso. Usa el PID como key. */

void (*ImprimirInfoAlgoritmosSustitucion)(void);
void (*QuitarDeEstructuraDeSeleccion) (uint32_t pid,uint16_t seg,uint16_t pag);
void (*AgregarPaginaAEstructuraSustitucion) (uint32_t pid,uint16_t seg,uint16_t pag);
void (*ActualizarEnEstructuraDeSustitucion) (uint32_t pid,uint16_t seg,uint16_t pag);
void (*RutinaSeleccionPaginaVictima) (t_segmento** tabla,uint32_t *pid,uint16_t *seg,uint16_t *pag);
/* FIN Variables Globales */

/* Interfaz MSP */
uint32_t crearSegmento(uint32_t pid, size_t size,t_msg_id* id);
t_msg_id destruirSegmento(uint32_t pid, uint32_t baseSegmento);
char* solicitarMemoria(uint32_t pid,uint32_t direccionLogica,uint32_t size,t_msg_id* id);
t_msg_id escribirMemoria(uint32_t pid,uint32_t direccionLogica,char* bytesAEscribir,uint32_t size);
/* FIN Interfaz MSP */

/* Funciones Privadas */
uint32_t marcoVacio(void);
char* string_uitoa(int number);
uint16_t primerEntradaLibre(t_segmento *tabla);
uint32_t rutinaSustitucion(uint32_t inPid,uint16_t seg,uint16_t pag);
void traerPaginaAMemoria(t_segmento *tabla,uint32_t pid,uint16_t seg,uint16_t pag);
t_segmento* traducirDireccion(uint32_t pid,uint32_t direccionLogica,uint16_t *numSegmento,uint16_t *numPagina,uint8_t *offset);
/* FIN Funciones Privadas */

#endif /* ADMINISTRADORDEMEMORIA_H_ */
