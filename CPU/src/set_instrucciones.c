/*
 * instrucciones-eso.c
 *
 *  Created on: 09/09/2014
 *      Author: matias
 */

#include "execution_unit.h"

/* Instrucciones de Usuario*/

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

	msp_memcpy(stack_top,registro(i),size);
	Registros.S += size;

}

void take (void) {

	int32_t size = fetch_numero();
	uint32_t i = fetch_registro();

	msp_memcpy(registro(i),stack_top-size,size);
	Registros.S -= size;

}

void xxxx (void) {

	Execution_State = FINISHED_THREAD;

}


/* Instrucciones Protegidas */

void innn (void) {

	t_msg *msg = id_message(NUMERIC_INPUT);

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

	msg = recibir_mensaje(Kernel);

	registro(A) = msg->argv[0];

	destroy_message(msg);

}

void innc (void) {

	t_msg *msg = argv_message(STRING_INPUT,1,registro(B));

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

	msg = recibir_mensaje(Kernel);

	escribir_memoria(registro(A),msg->stream,registro(B));

	destroy_message(msg);

}

void outn (void) {

	t_msg *msg = argv_message(STRING_OUTPUT,1,registro(A));//NUMERIC_OUTPUT

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

}

void outc (void) {

	char *buffer = solicitar_memoria(registro(A),registro(B));

	t_msg *msg = string_message(STRING_OUTPUT,buffer,1,registro(B));

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

}

void crea (void) {

	t_msg *msg;
	t_msg_id id;
	t_hilo *hilo_hijo = malloc(sizeof(t_hilo));

	memcpy(hilo_hijo,&Hilo,sizeof(t_hilo));

	++hilo_hijo->tid;
	hilo_hijo->puntero_instruccion = registro(B);
	hilo_hijo->kernel_mode = 0;
	hilo_hijo->base_stack = crear_segmento(stack_size,&id);

	if(id == OK_CREATE) {

		hilo_hijo->cursor_stack = hilo_hijo->base_stack + stack_size;
		msp_memcpy(hilo_hijo->base_stack,Registros.X,stack_size);

		msg = tcb_message(CPU_CREA,hilo_hijo,0);
	}
	else
		msg = id_message(CPU_CREA);//CPU_ABORT

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

}

void join (void) {

	t_msg *msg = argv_message(CPU_JOIN,1,registro(A));

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

}

void block (void) {

	t_msg *msg = argv_message(CPU_BLOCK,1,registro(B));

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

}

void wake (void) {

	t_msg *msg = argv_message(CPU_WAKE,1,registro(B));

	enviar_mensaje(Kernel,msg);

	destroy_message(msg);

}
