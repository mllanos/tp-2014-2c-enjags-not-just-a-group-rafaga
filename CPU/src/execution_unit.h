/*
 * funciones.h
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#ifndef EXECUTION_UNIT_H
#define EXECUTION_UNIT_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <utiles/utiles.h>
#include "set_instrucciones.h"
#define OPERATION_CODE_SIZE 4;

/*Definición de los tipos Registro*/
typedef struct {
	int32_t registros_programacion[5]; 	//A, B, C, D y E
	uint32_t M; 						//Base de segmento de código
	uint32_t P; 						//Puntero de instrucción
	uint32_t X; 						//Base del segmento de Stack
	uint32_t S; 						//Cursor de stack
	uint32_t K; 						//Kernel Mode
	uint32_t I; 						//PID
} __attribute__ ((__packed__)) t_registros_cpu;

enum {A,B,C,D,E} typedef t_registros_programacion;
/*FIN_Definición de los tipos Registro*/

/*Definición de t_hilo (Más adelante ponerlo en el header del kernel y sacarlo del header del panel*/
#ifndef T_HILO_
#define T_HILO_
typedef enum { NEW, READY, EXEC, BLOCK, EXIT } t_cola;

typedef struct {
	uint32_t pid;
	uint32_t tid;
	bool kernel_mode;
	uint32_t segmento_codigo;
	uint32_t segmento_codigo_size;
	uint32_t puntero_instruccion;
	uint32_t base_stack;
	uint32_t cursor_stack;
	int32_t registros[5];
	t_cola cola;
} __attribute__ ((__packed__)) t_hilo;
#endif
/*FIN_Definición de t_hilo*/

/*Variables Globales*/
int msp;
int kernel;
t_msg *buffer;
t_hilo hilo;
uint32_t quantum;
size_t instruccion_size;
t_registros_cpu registros;
/*FIN_Variables Globales*/

/*dummy*/
FILE *tcb;
/*dummy*/

/*Funciones de la UE (Unidad de Ejecución)*/
void obtener_siguiente_hilo (void);
void avanzar_puntero_instruccion(size_t desplazamiento);
void eu_actualizar_registros(void);
void eu_cargar_registros(void);
void eu_fetch_instruccion(void);
void eu_decode(void);
void eu_ejecutar(int retardo);
int fetch_operand(t_operandos tipo_operando);
/*FIN_Funciones de la UE (Unidad de Ejecución)*/

#endif /*EXECUTION_UNIT_H*/
