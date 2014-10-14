/*
 * instrucciones-eso.c
 *
 *  Created on: 09/09/2014
 *      Author: matias
 */

#include <stdlib.h>
#include "set_instrucciones.h"
#include "execution_unit.h"

void load (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';		//posicion del registro leido en el array registros

	registros.registros_programacion[i] = (int32_t) fetch_operand(NUMERO);

}

void getm (void) {

	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	t_msg *new_msg = msp_solicitar_memoria(registros.I, registros.M + registros.registros_programacion[j], REG_SIZE, MEM_REQUEST);
	//if(new_msg->header.id != ???) //falta elegir un msg_id
			;//abortar la ejecucion?
	registros.registros_programacion[i] = new_msg->argv[i]; //4bytes? preguntar qué es "memoria apuntada"
	destroy_message(new_msg);

}

void setm (void) {

	uint8_t cantidad = fetch_operand(NUMERO);
	uint32_t i = fetch_operand(REGISTRO) - 'A';
	uint32_t j = fetch_operand(REGISTRO) - 'A';

	memcpy(registros.registros_programacion + i,registros.registros_programacion + j, cantidad);

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
		perror("División por 0");

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

	if(registros.registros_programacion[A] == 0)
		registros.P = fetch_operand(DIRECCION);

}

void jpnz (void) {

	if(registros.registros_programacion[A])
		registros.P = fetch_operand(DIRECCION);

}

void inte (void) {

	//servicio_kernel(CPU_INTERRUPT,fetch_operand(DIRECCION));

}

void shif (void) {

	int n,i;

	n = fetch_operand(NUMERO);
	i = fetch_operand(REGISTRO) - 'A';

	if(n < 0)
		registros.registros_programacion[i] = registros.registros_programacion[i] << (n * -1); //REVISAR
	else if(n > 0) {
		int size = sizeof(uint32_t) * 8 - 1;
		registros.registros_programacion[i] = (registros.registros_programacion[i] >> n) & ~(((0x1 << size) >> n) << 1);
	}

}

void nopp (void) {

}

void eso_push (void) {

	uint32_t cantidad_bytes = fetch_operand(NUMERO);
	uint32_t i = fetch_operand(REGISTRO) - 'A';

	msp_escribir_memoria(registros.I,registros.X + registros.S,&registros.registros_programacion[i],cantidad_bytes);
	registros.S += cantidad_bytes;

}

void take (void) {

	uint32_t cantidad_bytes = fetch_operand(NUMERO);
	uint32_t i = fetch_operand(REGISTRO) - 'A';

	t_msg *new_msg = msp_solicitar_memoria(registros.I, registros.X + registros.S, cantidad_bytes, MEM_REQUEST); //por ahi conviene poner otro id
	registros.S -= cantidad_bytes;
	//if(new_msg->header.id != ???) //falta elegir un msg_id
		;//abortar la ejecucion?
	memcpy(registros.registros_programacion + i, new_msg->argv, cantidad_bytes);
	destroy_message(new_msg);

}

void xxxx (void) {

	//servicio_kernel(FINISHED_THREAD);
	exit(0); // Aca se muere el CPU? :|

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

	strcpy(tabla_instrucciones[18].mnemonico, "SHIF");
	tabla_instrucciones[18].rutina = shif;

	strcpy(tabla_instrucciones[19].mnemonico, "NOPP");
	tabla_instrucciones[19].rutina = nopp;

	strcpy(tabla_instrucciones[20].mnemonico, "PUSH");
	tabla_instrucciones[20].rutina = eso_push;

	strcpy(tabla_instrucciones[21].mnemonico, "TAKE");
	tabla_instrucciones[21].rutina = take;

	strcpy(tabla_instrucciones[22].mnemonico, "XXXX");
	tabla_instrucciones[22].rutina = xxxx;

	//INSTRUCCIONES PROTEGIDAS
/*
	tabla_instrucciones[23].mnemonico = "MALC";
	tabla_instrucciones[23].rutina = malc;

	tabla_instrucciones[24].mnemonico = "FREE";
	tabla_instrucciones[24].rutina = eso_free;

	tabla_instrucciones[25].mnemonico = "INNN";
	tabla_instrucciones[25].rutina = innn;

	tabla_instrucciones[26].mnemonico = "INNC";
	tabla_instrucciones[26].rutina = innc;

	tabla_instrucciones[27].mnemonico = "OUTN";
	tabla_instrucciones[27].rutina = outn;

	tabla_instrucciones[28].mnemonico = "OUTC";
	tabla_instrucciones[28].rutina = outc;

	tabla_instrucciones[29].mnemonico = "CREA";
	tabla_instrucciones[29].rutina = crea;

	tabla_instrucciones[30].mnemonico = "JOIN";
	tabla_instrucciones[30].rutina = join;

	tabla_instrucciones[31].mnemonico = "BLOK";
	tabla_instrucciones[31].rutina = blok;

	tabla_instrucciones[32].mnemonico = "WAKE";
	tabla_instrucciones[32].rutina = wake;
*/
}
