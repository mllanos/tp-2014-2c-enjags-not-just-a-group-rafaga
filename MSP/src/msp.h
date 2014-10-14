/*
 * msp.h
 *
 *  Created on: 05/09/2014
 *      Author: utnso
 */

#ifndef MSP_H_
#define MSP_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <panel/panel.h>
#include <commons/log.h>
#include <utiles/utiles.h>
#include <commons/config.h>

#define K 1024
#define M 1048576
#define PAG_SIZE 256
#define NUM_SEG_MAX 4*K
#define NUM_PAG_MAX NUM_SEG_MAX
#define SEG_MAX_SIZE 1048576

typedef uint8_t bit;

/* Estructuras */
typedef struct {
	bit ocupado;	/* 1: ocupado - 0: disponible */
	char marco[PAG_SIZE];
} __attribute__ ((__packed__)) t_marco;

typedef struct {
	bit bitPresencia;		/* 1: está en memoria - 0: está en swap */
	uint32_t numMarco;		/* Número de marco: indica la posición en el array MemoriaPrincipal */
} __attribute__ ((__packed__)) t_pagina;

typedef struct {
	uint32_t limite;
	uint32_t bytesOcupados;
	t_pagina *tablaPaginas;
} __attribute__ ((__packed__)) t_segmento;
/* FIN Estructuras */

/* Variables Globales */
int  Puerto;
t_log *Logger;
uint32_t MaxMem;
uint32_t MaxSwap;
char *SwapPath;
char *AlgoritmoSustitucion;
/* FIN Variables Globales */

void inicializarMSP(char* swapPath);

#endif /* MSP_H_ */
