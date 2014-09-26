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
#include <panel/panel.h>
#include <utiles/utiles.h>
#include <commons/collections/list.h>
#include "set_instrucciones.h"

#define OPERATION_CODE_SIZE 4

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

/*Variables Globales*/
int msp;
int kernel;
t_hilo hilo;
uint16_t quantum;
size_t instruccion_size;
t_registros_cpu registros;
/*FIN_Variables Globales*/

/*Funciones de la UE (Unidad de Ejecución)*/
void obtener_siguiente_hilo (void);
void avanzar_puntero_instruccion(size_t desplazamiento);
void eu_actualizar_registros(void);
void eu_cargar_registros(void);
void eu_fetch_instruccion(void);
void eu_decode(void);
void eu_ejecutar(int retardo);
int fetch_operand(t_operandos tipo_operando);
void devolver_hilo(void);
t_msg* msp_solicitar_memoria(uint32_t pid,uint32_t direccion_logica,uint32_t size, t_msg_id id);
t_msg* msp_escribir_memoria(uint32_t pid,uint32_t direccion_logica,void *bytes_a_escribir,uint32_t size);
//void servicio_kernel(void);
/*FIN_Funciones de la UE (Unidad de Ejecución)*/

/**
 * Debe invocarse cada vez se vaya a ejecutar una instrucción.
 * Por ejemplo: ejecucion_instruccion("ABCD", "soy", 1, "parametro");
 *
 * @param  mnemonico  Nombre de la instrucción a ejecutar.
 * @param  parametros  Parametros de la instrucción a ejecutar.
 */
void ejecucion_instruccion(char* mnemonico, t_list* parametros);

/**
 * Debe invocarse cada vez que ocura algún cambio en alguno de los
 * registros de la CPU (una vez por instruccion a ejecutar, luego de
 * llamar a ejecucion_instruccion()).
 *
 * @param  registros  Estructura conteniendo cada uno de los registros de la CPU.
 */
void cambio_registros(t_registros_cpu registros);

#endif /*EXECUTION_UNIT_H*/
