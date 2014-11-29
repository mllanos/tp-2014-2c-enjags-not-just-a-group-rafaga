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
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <panel/cpu.h>

#define MAX_SHIF 31
#define PID Registros.I
#define OPERATION_CODE_SIZE 4
#define stack_top Registros.S
#define stack_size (Registros.S - Registros.X)
#define PIDKM (KernelMode == false? Registros.I : 0)
#define program_counter (Registros.M + Registros.P)

#define LOG_PATH "panel/cpu.log"

typedef enum {A,B,C,D,E} t_registros_programacion;
typedef enum {REGISTRO,NUMERO,DIRECCION} t_operandos;


/*Variables Globales*/
int MSP;
int Kernel;
t_hilo Hilo;
bool KernelMode;
uint16_t Quantum;
t_msg *Kernel_Msg;
t_list *Parametros;
t_msg_id Execution_State;
uint32_t Instruction_size;
t_registros_cpu Registros;
void *MapRegistros['X'-'A'+1];
t_dictionary *SetInstruccionesDeUsuario;
t_dictionary *SetInstruccionesProtegidas;

t_log *Logger;

void (*Instruccion)(void);
/*FIN_Variables Globales*/

/* Funciones Macro */
#define registro(n) *((int32_t*) MapRegistros[n])

#define fetch_registro() fetch_operand(REGISTRO) - 'A'
#define fetch_numero() (int32_t) fetch_operand(NUMERO)
#define fetch_direccion() (uint32_t) fetch_operand(DIRECCION)

#define msp_memcpy(DEST,SOURCE,SIZE) escribir_memoria(PID,DEST,solicitar_memoria(PID,SOURCE,SIZE),SIZE)

#define avanzar_puntero_instruccion() (Registros.P += Instruction_size)
#define eu_fetch_instruccion(OC) OC = solicitar_memoria(PIDKM,program_counter,Instruction_size = OPERATION_CODE_SIZE)
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

uint32_t crear_segmento(uint32_t size);
void destruir_segmento(uint32_t baseSegmento);
char* solicitar_memoria(uint32_t pid,uint32_t direccionLogica,uint32_t size);
void escribir_memoria(uint32_t pid,uint32_t direccionLogica,char *bytesAEscribir,uint32_t size);
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

void malc (void);
void innn (void);
void innc (void);
void outn (void);
void outc (void);
void crea (void);
void join (void);
void blok (void);
void wake (void);
void eso_free (void);
/*FIN Set de Instrucciones */

#endif /*EXECUTION_UNIT_H*/
