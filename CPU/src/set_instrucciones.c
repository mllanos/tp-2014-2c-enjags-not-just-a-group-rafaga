/*
 * instrucciones-eso.c
 *
 *  Created on: 09/09/2014
 *      Author: matias
 */

#include "set_instrucciones.h"
/*
void inicializar_tabla_instrucciones(void){

	tabla_instrucciones[0].mnemonico = "LOAD";
	tabla_instrucciones[0].rutina = load;

	tabla_instrucciones[1].mnemonico = "GETM";
	tabla_instrucciones[1].rutina = getm;

	tabla_instrucciones[2].mnemonico = "SETM";
	tabla_instrucciones[2].rutina = setm;

	tabla_instrucciones[3].mnemonico = "MOVR";
	tabla_instrucciones[3].rutina = movr;

	tabla_instrucciones[4].mnemonico = "ADDR";
	tabla_instrucciones[4].rutina = addr;

	tabla_instrucciones[5].mnemonico = "SUBR";
	tabla_instrucciones[5].rutina = subr;

	tabla_instrucciones[6].mnemonico = "MULR";
	tabla_instrucciones[6].rutina = mulr;

	tabla_instrucciones[7].mnemonico = "MODR";
	tabla_instrucciones[7].rutina = modr;

	tabla_instrucciones[8].mnemonico = "DIVR";
	tabla_instrucciones[8].rutina = divr;

	tabla_instrucciones[9].mnemonico = "INCR";
	tabla_instrucciones[9].rutina = incr;

	tabla_instrucciones[10].mnemonico = "DECR";
	tabla_instrucciones[10].rutina = decr;

	tabla_instrucciones[11].mnemonico = "COMP";
	tabla_instrucciones[11].rutina = comp;

	tabla_instrucciones[12].mnemonico = "CGEQ";
	tabla_instrucciones[12].rutina = cgeq;

	tabla_instrucciones[13].mnemonico = "CLEQ";
	tabla_instrucciones[13].rutina = cleq;

	tabla_instrucciones[14].mnemonico = "GOTO";
	tabla_instrucciones[14].rutina = eso_goto;

	tabla_instrucciones[15].mnemonico = "JMPZ";
	tabla_instrucciones[15].rutina = jmpz;

	tabla_instrucciones[16].mnemonico = "JPNZ";
	tabla_instrucciones[16].rutina = jpnz;

	tabla_instrucciones[17].mnemonico = "INTE";
	tabla_instrucciones[17].rutina = inte;

	tabla_instrucciones[18].mnemonico = "FLCL";
	tabla_instrucciones[18].rutina = flcl;

	tabla_instrucciones[19].mnemonico = "SHIF";
	tabla_instrucciones[19].rutina = shif;

	tabla_instrucciones[20].mnemonico = "NOPP";
	tabla_instrucciones[20].rutina = nopp;

	tabla_instrucciones[21].mnemonico = "PUSH";
	tabla_instrucciones[21].rutina = eso_push;

	tabla_instrucciones[22].mnemonico = "TAKE";
	tabla_instrucciones[22].rutina = take;

	tabla_instrucciones[23].mnemonico = "XXXX";
	tabla_instrucciones[23].rutina = xxxx;

	//INSTRUCCIONES PROTEGIDAS

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

}
*/
void load (void){

	uint32_t numero_registro = fetch_operand(REGISTRO) - 'A';		//posicion del registro leido en el array registros

	registros[numero_registro] = (int32_t) fetch_operand(NUMERO);

}
