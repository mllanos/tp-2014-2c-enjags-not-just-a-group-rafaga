/*
 * funciones.h
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#ifndef EXECUTION_UNIT_H
#define EXECUTION_UNIT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <utiles/utiles.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <panel/cpu_log.h>

#define MAX_SHIF 31
#define PID Registros.I
#define OPERATION_CODE_SIZE 4
#define stack_top (Registros.X + Registros.S)
#define stack_size (Registros.S - Registros.X)
#define program_counter (Registros.M + Registros.P)


typedef enum {A,B,C,D,E} t_registros_programacion;
typedef enum {REGISTRO,NUMERO,DIRECCION} t_operandos;

/*Variables Globales*/
int MSP;
int Kernel;
t_hilo Hilo;
uint16_t Quantum;
t_msg_id Execution_State;
uint32_t Instruction_size;
t_registros_cpu Registros;
t_dictionary *SetInstruccionesDeUsuario;
t_dictionary *SetInstruccionesProtegidas;

void (*Instruccion)(void);
/*FIN_Variables Globales*/

/* Funciones Macro */
#define registro(n) Registros.registros_programacion[n]

#define fetch_registro() fetch_operand(REGISTRO) - 'A'
#define fetch_numero() (int32_t) fetch_operand(NUMERO)
#define fetch_direccion() (uint32_t) fetch_operand(DIRECCION)

#define msp_memcpy(DEST,SOURCE,SIZE) escribir_memoria(DEST,solicitar_memoria(SOURCE,SIZE),SIZE)

#define avanzar_puntero_instruccion() (Registros.P += Instruction_size)
#define eu_fetch_instruccion(OC) OC = solicitar_memoria(program_counter,Instruction_size = OPERATION_CODE_SIZE)
/* FIN Funciones Macro */

/*Funciones de la UE (Unidad de Ejecución)*/
void inicializar_tabla_instrucciones(void);

void devolver_hilo(void);
void eu_cargar_registros(void);
void eu_actualizar_registros(void);
void obtener_siguiente_hilo (void);
void eu_decode(char *operation_code);
int fetch_operand(t_operandos tipo_operando);
void eu_ejecutar(char *operation_code,uint32_t retardo);

uint32_t crear_segmento(uint32_t size,t_msg_id *id);
char* solicitar_memoria(uint32_t direccionLogica,uint32_t size);
t_msg_id escribir_memoria(uint32_t direccionLogica,char *bytesAEscribir,uint32_t size);
//void servicio_kernel(void);
/*FIN_Funciones de la UE (Unidad de Ejecución)*/

/* Set de Instrucciones */
void load (void);
void getm (void);
void setm (void);
void movr (void);
void addr (void);
void subr (void);
void mulr (void);
void modr (void);
void divr (void);
void incr (void);
void decr (void);
void comp (void);
void cgeq (void);
void cleq (void);
void jmpz (void);
void jpnz (void);
void inte (void);
void shif (void);
void nopp (void);
void take (void);
void xxxx (void);
void eso_goto (void);
void eso_push (void);
/*FIN Set de Instrucciones */

#endif /*EXECUTION_UNIT_H*/
