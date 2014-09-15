/*
 * execution_unit.c
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#include "execution_unit.h"

/*Variables Locales*/
static char oc_instruccion[5];											//operation code
static int cursor_tabla,fin_tabla;
/*FIN_Variables Locales*/

/*dummy*/
FILE *tcb;
/*dummy*/

void obtener_siguiente_hilo (void) {

	//cargar tcb en hilo
	quantum = 5;

}

void avanzar_puntero_instruccion(size_t desplazamiento){

	registros.P += instruccion_size;

}

void eu_cargar_registros(void){

	int i;
	for(i = 0;i < 5; ++i)
		registros.registros_programacion[i] = hilo.registros[i];

	registros.flags = hilo.flags;
	registros.I = hilo.pid;
	registros.X = hilo.base_stack;
	registros.K = hilo.kernel_mode;
	registros.S = hilo.cursor_stack;
	registros.M = hilo.segmento_codigo; 								//Base de segmento de código
	registros.P = hilo.puntero_instruccion;								//Puntero de instrucción

}

void eu_actualizar_registros(void){

	int i;
	for(i = 0;i < 5; ++i)
		hilo.registros[i] = registros.registros_programacion[i];

	hilo.flags = registros.flags;
	hilo.cursor_stack = registros.S;
	hilo.puntero_instruccion = registros.P;

}

void eu_fetch_instruccion(void){

	//recibir de msp y guardar en oc_instruccion
	instruccion_size = OPERATION_CODE_SIZE;

	fread(oc_instruccion,instruccion_size,1,tcb);

}

void eu_decode(void){

	if(registros.K == 0)
		fin_tabla = 23;
	else
		fin_tabla = 33;

	for(cursor_tabla = 0;cursor_tabla <= fin_tabla && strcmp(oc_instruccion,tabla_instrucciones[cursor_tabla].mnemonico);++cursor_tabla )
		;

	//if(cursor_tabla > fin_tabla)
		;//hacer algo con las instrucciones inválidas (no existe o no tiene privilegios para ejecutarla)

}

void eu_ejecutar(int retardo){

	usleep(retardo * 1000);						//Por ahora queda el usleep, después hay que mejorar el método para generar el delay
	tabla_instrucciones[cursor_tabla].rutina();

}

int fetch_operand(t_operandos tipo_operando){

	switch(tipo_operando){
	case REGISTRO:;
		unsigned char registro;
		//recibir un char de msp y guardar en registro
		fread(&registro,sizeof(char),1,tcb);
		instruccion_size += 1;
		return (char) registro;
		break;
	case NUMERO:;
		int32_t numero;
		//recibir un int32 de msp y guardar en direccion
		fread(&numero,sizeof(int32_t),1,tcb);
		instruccion_size += 4;
		return (int32_t) numero;
		break;
	case DIRECCION:;
		uint32_t direccion;
		//recibir uint32 de msp y guardar en direccion
		fread(&direccion,sizeof(uint32_t),1,tcb);
		instruccion_size += 4;
		return (uint32_t) direccion;
		break;
	}

	return 4000000000;	//Para que no hinche con el warning, después hay que borrarlo

}

/*dummys*/
int conectar_a_kernel(void){
	return 0;
}

int conectar_a_msp(void){

	tcb = fopen("arithmetics.bc","r+");

	return 0;
}
/*dummys*/
