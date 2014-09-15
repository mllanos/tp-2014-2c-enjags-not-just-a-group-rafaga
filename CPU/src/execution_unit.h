/*
 * funciones.h
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#ifndef EXECUTION_UNIT_H
#define EXECUTION_UNIT_H

#include <stdint.h>
#include <stddef.h>
#include "panel.h"
#include "set_instrucciones.h"
#define OPERATION_CODE_SIZE 4;

t_hilo hilo;
uint32_t quantum;
size_t instruccion_size;

typedef struct {
	int32_t registros_programacion[5]; //A, B, C, D y E
	uint32_t M; //Base de segmento de código
	uint32_t P; //Puntero de instrucción
	uint32_t X; //Base del segmento de Stack
	uint32_t S; //Cursor de stack
	uint32_t K; //Kernel Mode
	uint32_t I; //PID
	uint32_t flags;			//Agregar a tcb? de donde los saco, que hago con los flags!?
} t_registros_cpu;

enum {A,B,C,D,E} typedef t_registros_programacion;


t_registros_cpu registros;



void obtener_siguiente_hilo (void);
void avanzar_puntero_instruccion(size_t desplazamiento);
void eu_actualizar_registros(void);
void eu_cargar_registros(void);
void eu_fetch_instruccion(void);
void eu_decode(void);
void eu_ejecutar(int retardo);
int fetch_operand(t_operandos tipo_operando);



int conectar_a_kernel(void);
int conectar_a_msp(void);



#endif /*EXECUTION_UNIT_H*/
