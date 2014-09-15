/*
 * instrucciones-eso.c
 *
 *  Created on: 09/09/2014
 *      Author: matias
 */

#include "set_instrucciones.h"
#include "execution_unit.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void load (void){

	uint32_t i = fetch_operand(REGISTRO) - 'A';		//posicion del registro leido en el array registros

	registros.registros_programacion[i] = (int32_t) fetch_operand(NUMERO);

}

void getm (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	//registros.registros_programacion[i] = msp_solicitar_memoria(registros.I,registros.registros_programacion[j],sizeof(int32_t));	//4bytes? preguntar qué es "memoria apuntada"

}

void setm (void) {

	size_t operand_size = fetch_operand(NUMERO);
	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	//msp_escribir_memoria(registros.I,registros.registros_programacion[i],registros.registros_programacion[j],operand_size);

}

void movr (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	registros.registros_programacion[i] = registros.registros_programacion[j];

}

void addr (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	registros.registros_programacion[A] = registros.registros_programacion[i] + registros.registros_programacion[j];

}

void subr (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	registros.registros_programacion[A] = registros.registros_programacion[i] - registros.registros_programacion[j];

}

void mulr (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	registros.registros_programacion[A] = registros.registros_programacion[i] * registros.registros_programacion[j];

}

void modr (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	registros.registros_programacion[A] = registros.registros_programacion[i] % registros.registros_programacion[j];

}

void divr (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	if(registros.registros_programacion[j])
		registros.registros_programacion[A] = registros.registros_programacion[i] / registros.registros_programacion[j];
	else
		registros.flags = 0;						//setear flag ZERO_DIV, cambiar por el valor que corresponde

}

void incr (void) {

	++registros.registros_programacion[fetch_operand(REGISTRO) - 'A'];

}

void decr (void) {

	--registros.registros_programacion[fetch_operand(REGISTRO) - 'A'];

}

void comp (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	registros.registros_programacion[A] = registros.registros_programacion[i] == registros.registros_programacion[j];

}

void cgeq (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	registros.registros_programacion[A] = registros.registros_programacion[i] >= registros.registros_programacion[j];

}

void cleq (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	registros.registros_programacion[A] = registros.registros_programacion[i] <= registros.registros_programacion[j];

}

void eso_goto (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';

	registros.P = registros.registros_programacion[i];

}

void jmpz (void) {

	int32_t i = fetch_operand(REGISTRO) - 'A';

	if(registros.registros_programacion[A] == 0)				//se puede optimizar si hago el if antes del fetch; la única consideración es que deberia adelantar el
		registros.P = registros.registros_programacion[i];		//puntero de instruccion a mano, para saltear ese parametro que nunca leería

}

void jpnz (void) {

	int32_t i = fetch_operand(REGISTRO) - 'A';

	if(registros.registros_programacion[A])
		registros.P = registros.registros_programacion[i];

}

void inte (void) {

	//implementacion pendiente

}

void flcl (void) {

	registros.flags = 0;

}

void shif (void) {

	//implementacion pendiente

}

void nopp (void) {

}

void eso_push (void) {

	size_t cantidad_bytes = fetch_operand(NUMERO);
	uint32_t i = fetch_operand(REGISTRO) - 'A';

	//msp_escribir_memoria(registros.I,registros.S,registros.registros_programacion[i],cantidad_bytes);
	registros.S += cantidad_bytes;

}

void take (void) {

	size_t cantidad_bytes = fetch_operand(NUMERO);
	uint32_t i = fetch_operand(REGISTRO) - 'A';

	//msp_solicitar_memoria(registros.I,registros.registros_programacion[i],cantidad_bytes);
	registros.S -= cantidad_bytes;

}

void xxxx (void) {

	/*dummy*/
	imprimir_tcb();
	exit(0);
	/*dummy*/
	//decir_al_kernel_que_el_hilo_finalizo_la_ejecucion

}

void inicializar_tabla_instrucciones(void){

	strcpy(tabla_instrucciones[0].mnemonico,"LOAD");
	tabla_instrucciones[0].rutina = load;

	strcpy(tabla_instrucciones[1].mnemonico, "GETM");
	tabla_instrucciones[1].rutina = getm;

	strcpy(tabla_instrucciones[2].mnemonico, "SETM");
	tabla_instrucciones[2].rutina = setm;

	strcpy(tabla_instrucciones[3].mnemonico, "MOVR");
	tabla_instrucciones[3].rutina = movr;

	strcpy(tabla_instrucciones[4].mnemonico, "ADDR");
	tabla_instrucciones[4].rutina = addr;

	strcpy(tabla_instrucciones[5].mnemonico, "SUBR");
	tabla_instrucciones[5].rutina = subr;

	strcpy(tabla_instrucciones[6].mnemonico, "MULR");
	tabla_instrucciones[6].rutina = mulr;

	strcpy(tabla_instrucciones[7].mnemonico, "MODR");
	tabla_instrucciones[7].rutina = modr;

	strcpy(tabla_instrucciones[8].mnemonico, "DIVR");
	tabla_instrucciones[8].rutina = divr;

	strcpy(tabla_instrucciones[9].mnemonico, "INCR");
	tabla_instrucciones[9].rutina = incr;

	strcpy(tabla_instrucciones[10].mnemonico, "DECR");
	tabla_instrucciones[10].rutina = decr;

	strcpy(tabla_instrucciones[11].mnemonico, "COMP");
	tabla_instrucciones[11].rutina = comp;

	strcpy(tabla_instrucciones[12].mnemonico, "CGEQ");
	tabla_instrucciones[12].rutina = cgeq;

	strcpy(tabla_instrucciones[13].mnemonico, "CLEQ");
	tabla_instrucciones[13].rutina = cleq;

	strcpy(tabla_instrucciones[14].mnemonico, "GOTO");
	tabla_instrucciones[14].rutina = eso_goto;

	strcpy(tabla_instrucciones[15].mnemonico, "JMPZ");
	tabla_instrucciones[15].rutina = jmpz;

	strcpy(tabla_instrucciones[16].mnemonico, "JPNZ");
	tabla_instrucciones[16].rutina = jpnz;

	strcpy(tabla_instrucciones[17].mnemonico, "INTE");
	tabla_instrucciones[17].rutina = inte;

	strcpy(tabla_instrucciones[18].mnemonico, "FLCL");
	tabla_instrucciones[18].rutina = flcl;

	strcpy(tabla_instrucciones[19].mnemonico, "SHIF");
	tabla_instrucciones[19].rutina = shif;

	strcpy(tabla_instrucciones[20].mnemonico, "NOPP");
	tabla_instrucciones[20].rutina = nopp;

	strcpy(tabla_instrucciones[21].mnemonico, "PUSH");
	tabla_instrucciones[21].rutina = eso_push;

	strcpy(tabla_instrucciones[22].mnemonico, "TAKE");
	tabla_instrucciones[22].rutina = take;

	strcpy(tabla_instrucciones[23].mnemonico, "XXXX");
	tabla_instrucciones[23].rutina = xxxx;

	//INSTRUCCIONES PROTEGIDAS
/*
	tabla_instrucciones[24].mnemonico = "MALC";
	tabla_instrucciones[24].rutina = malc;

	tabla_instrucciones[25].mnemonico = "FREE";
	tabla_instrucciones[25].rutina = eso_free;

	tabla_instrucciones[26].mnemonico = "INNN";
	tabla_instrucciones[26].rutina = innn;

	tabla_instrucciones[27].mnemonico = "INNC";
	tabla_instrucciones[27].rutina = innc;

	tabla_instrucciones[28].mnemonico = "OUTN";
	tabla_instrucciones[28].rutina = outn;

	tabla_instrucciones[29].mnemonico = "OUTC";
	tabla_instrucciones[29].rutina = outc;

	tabla_instrucciones[30].mnemonico = "CREA";
	tabla_instrucciones[30].rutina = crea;

	tabla_instrucciones[31].mnemonico = "JOIN";
	tabla_instrucciones[31].rutina = join;

	tabla_instrucciones[32].mnemonico = "BLOK";
	tabla_instrucciones[32].rutina = blok;

	tabla_instrucciones[33].mnemonico = "WAKE";
	tabla_instrucciones[33].rutina = wake;
*/
}












