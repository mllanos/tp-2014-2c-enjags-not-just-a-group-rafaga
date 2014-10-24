/*
 * instrucciones-eso.c
 *
 *  Created on: 09/09/2014
 *      Author: matias
 */

#include "execution_unit.h"

void load (void) {

	Registros.registros_programacion[fetch_registro()] = fetch_numero();

}

void getm (void) {

	uint32_t i = fetch_registro();
	uint32_t j = fetch_registro();

	registro(i) = atoi(solicitar_memoria(registro(j),sizeof(uint32_t)));

}

void setm (void) {

	uint8_t size = fetch_numero();
	uint32_t i = fetch_registro();
	uint32_t j = fetch_registro();

	escribir_memoria(registro(i),string_itoa(registro(j)),size);

}

void movr (void) {

	uint32_t i = fetch_registro();
	uint32_t j = fetch_registro();

	registro(i) = registro(j);

}

void addr (void) {

	registro(A) = registro(fetch_registro()) + registro(fetch_registro());

}

void subr (void) {

	uint32_t i = fetch_registro();
	uint32_t j = fetch_registro();

	registro(A) = registro(i) - registro(j);

}

void mulr (void) {

	registro(A) = registro(fetch_registro()) * registro(fetch_registro());

}

void modr (void) {

	uint32_t i = fetch_registro();
	uint32_t j = fetch_registro();

	registro(A) = registro(i) % registro(j);

}

void divr (void) {

	uint32_t i = fetch_registro();
	uint32_t j = fetch_registro();

	if(registro(j))
		registro(A) = registro(i) / registro(j);
	else
		perror("División por 0");

}

void incr (void) {

	++registro(fetch_registro());

}

void decr (void) {

	--registro(fetch_registro());

}

void comp (void) {

	registro(A) = registro(fetch_registro()) == registro(fetch_registro());

}

void cgeq (void) {

	uint32_t i = fetch_registro();
	uint32_t j = fetch_registro();

	registro(A) = registro(i) >= registro(j);

}

void cleq (void) {

	uint32_t i = fetch_registro();
	uint32_t j = fetch_registro();

	registro(A) = registro(i) <= registro(j);

}

void eso_goto (void) {

	Registros.P = registro(fetch_registro());

}

void jmpz (void) {

	if(registro(A) == 0)
		Registros.P = fetch_direccion();

}

void jpnz (void) {

	if(registro(A))
		Registros.P = fetch_direccion();

}

void inte (void) {

	//mando un mensaje con la interrupcion, y despues el hilo? o mando todo junto? arreglar con mario
	//servicio_kernel(CPU_INTERRUPT,fetch_direccion());
	Quantum = 0;
	t_msg *msg = argv_message(CPU_INTERRUPT,1,fetch_direccion());

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

}

void shif (void) {

	int8_t n = fetch_numero();
	uint8_t i = fetch_registro();

	if(n < 0)
		registro(i) = registro(i) << (n * -1);
	else if(n > 0)
		registro(i) = (registro(i) >> n) & ~(((0x1 << MAX_SHIF) >> n) << 1); //REVISAR

}

void nopp (void) {

}

void eso_push (void) {

	int32_t size = fetch_numero();
	uint32_t i = fetch_registro();

	msp_memcpy(STACK_TOP,registro(i),size);
	Registros.S += size;

}

void take (void) {

	int32_t size = fetch_numero();
	uint32_t i = fetch_registro();

	msp_memcpy(registro(i),STACK_TOP-size,size);
	Registros.S -= size;

}

void xxxx (void) {

	Execution_State = FINISHED_THREAD;

}
