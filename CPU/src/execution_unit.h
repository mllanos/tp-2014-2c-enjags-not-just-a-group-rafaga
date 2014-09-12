/*
 * funciones.h
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#ifndef EXECUTION_UNIT_H
#define EXECUTION_UNIT_H

#define OPERATION_CODE_SIZE 4;

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

#endif /*EXECUTION_UNIT_H*/
