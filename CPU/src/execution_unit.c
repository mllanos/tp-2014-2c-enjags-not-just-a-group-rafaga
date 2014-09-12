/*
 * execution_unit.c
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#include "execution_unit.h"
#include "set_instrucciones.h"
#include "panel.h"

char oc_instruccion[5];			//operation code
int cursor_tabla,fin_tabla;
t_registros_cpu registros;
t_instruccion tabla_instrucciones[34];

void avanzar_puntero_instruccion(size_t desplazamiento){

	registros.P += instruccion_size;

}

void eu_actualizar_registros(void){

	int i;
	for(i = 0;i < 5; ++i)
		registros.registros_programacion[i] = hilo.registros[i];

	hilo.puntero_instruccion = registros.P;
	hilo.cursor_stack = registros.S;

}

void eu_cargar_registros(void){

	int i;
	for(i = 0;i < 5; ++i)
		registros.registros_programacion[i] = hilo.registros[i];

	registros.M = hilo.segmento_codigo; //Base de segmento de código
	registros.P = hilo.puntero_instruccion; //Puntero de instrucción
	registros.X = hilo.base_stack;
	registros.S = hilo.cursor_stack;
	registros.K = hilo.kernel_mode;
	registros.I = hilo.pid;

}

void eu_fetch_instruccion(void){

	//recibir de msp y guardar en oc_instruccion
	instruccion_size = 4;

}

void eu_decode(void){

	if(registros.K == 0)
		fin_tabla = 23;
	else
		fin_tabla = 33;

	for(cursor_tabla = 0;cursor_tabla <= fin_tabla && strcmp(oc_instruccion,tabla_instrucciones[cursor_tabla].mnemonico);++cursor_tabla )
		;//validar instruccion invalida;

	//if(cursor_tabla > fin_tabla)
		;//hacer algo con las instrucciones inválidas (no existe o no tiene privilegios para ejecutarla)

}

void eu_ejecutar(void){

	tabla_instrucciones[cursor_tabla].rutina();

}

int fetch_operand(t_operandos tipo_operando){

	switch(tipo_operando){
	case REGISTRO:
		unsigned char registro;
		//recibir un char de msp y guardar en registro
		instruccion_size += 1;
		return (char) registro;
		break;
	case NUMERO:
		int32_t numero;
		//recibir un int32 de msp y guardar en direccion
		instruccion_size += 4;
		return (int32_t) numero;
		break;
	case DIRECCION:
		uint32_t direccion;
		//recibir uint32 de msp y guardar en direccion
		instruccion_size += 4;
		return (uint32_t) direccion;
		break;
	}

	return 4000000000;

}
