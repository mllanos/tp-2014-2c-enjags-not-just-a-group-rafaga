/*
 * instrucciones-eso.c
 *
 *  Created on: 09/09/2014
 *      Author: matias
 */

#include <commons/collections/list.h>
#include "execution_unit.h"

static t_list *alloc_segs;

bool match(void* parametro) {

	uint32_t dir = *((uint32_t*) parametro);

	return registro(A) == dir;

}

/* Instrucciones de Usuario*/

void load (void) {

	uint8_t i = fetch_registro();
	int32_t val = fetch_numero();

	if(Execution_State != CPU_ABORT) {
		registro(i) = val;
	}
}

void getm (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT) {
		char *buffer = solicitar_memoria(registro(j),1);

		if(buffer != NULL) {
			memcpy(&registro(i),buffer,1);
			free(buffer);
		}
	}
}

void setm (void) {

	int32_t size = fetch_numero();
	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT) {
		char *buffer = malloc(size+1);
		memcpy(buffer,&registro(j),size);
		buffer[size] = '\0';

		escribir_memoria(registro(i),buffer,size);
	}
}

void movr (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT)
		registro(i) = registro(j);
}

void addr (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT)
		registro(A) = registro(i) + registro(j);
}

void subr (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT)
		registro(A) = registro(i) - registro(j);
}

void mulr (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT)
		registro(A) = registro(i) * registro(j);
}

void modr (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT)
		registro(A) = registro(i) % registro(j);
}

void divr (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT) {
		if(registro(j))
			registro(A) = registro(i) / registro(j);
		else {
			Quantum = 0;
			KernelMode = false;
			Execution_State = CPU_ABORT;
			printf("ERROR: ZERO_DIV");
		}
	}
}

void incr (void) {

	uint8_t i = fetch_registro();
	if(Execution_State != CPU_ABORT)
		++registro(i);
}

void decr (void) {

	uint8_t i = fetch_registro();
	if(Execution_State != CPU_ABORT)
		--registro(i);
}

void comp (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT)
		registro(A) = registro(i) == registro(j);
}

void cgeq (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT)
		registro(A) = registro(i) >= registro(j);
}

void cleq (void) {

	uint8_t i = fetch_registro();
	uint8_t j = fetch_registro();

	if(Execution_State != CPU_ABORT)
		registro(A) = registro(i) <= registro(j);
}

void eso_goto (void) {

	uint8_t i = fetch_registro();
	if(Execution_State != CPU_ABORT)
		Registros.P = registro(i) - Instruction_size;
}

void jmpz (void) {

	uint8_t dir = fetch_direccion();

	if(registro(A) == 0 && Execution_State != CPU_ABORT)
		Registros.P = dir;
}

void jpnz (void) {

	uint8_t dir = fetch_direccion();

	if(registro(A) && Execution_State != CPU_ABORT)
		Registros.P = dir;
}

void inte (void) {

	Quantum = 0;
	Kernel_Msg = tcb_message(CPU_INTERRUPT,&Hilo,1,fetch_direccion());
}

void shif (void) {

	int32_t n = fetch_numero();
	uint8_t i = fetch_registro();

	if(Execution_State != CPU_ABORT) {
		if(n < 0)
			registro(i) = registro(i) << (n * -1);
		else if(n > 0)
			registro(i) = registro(i) >> n;
		//según los ayudantes con el >> alcanza, por las dudas guardo la solución correcta: (registro(i) >> n) & ~(((0x1 << MAX_SHIF) >> n) << 1);
	}
}

void nopp (void) {

}

void eso_push (void) {

	int32_t size = fetch_numero();
	char *buffer = malloc(size+1);

	memcpy(buffer,&registro(fetch_registro()),size);

	buffer[size] = '\0';

	escribir_memoria(stack_top,buffer,size);

	Registros.S += size;
}

void take (void) {

	int32_t size = fetch_numero();

	char *buffer = solicitar_memoria(stack_top-size,size);

	if(buffer != NULL) {
		memcpy(&registro(fetch_registro()),buffer,size);
		Registros.S -= size;
		free(buffer);
	}
}

void xxxx (void) {

	Quantum = 0;
	KernelMode = false;
	Execution_State = FINISHED_THREAD;
}


/* Instrucciones Protegidas */

void malc (void) {

	uint32_t *aux = malloc(sizeof(uint32_t));
	registro(A) = crear_segmento(registro(A));

	if(Execution_State != CPU_ABORT) {
		memcpy(aux,&registro(A),sizeof(uint32_t));
		list_add(alloc_segs,aux);
	}
}

void eso_free (void) {

	uint32_t *aux = list_remove_by_condition(alloc_segs,match);

	if(aux) {
		destruir_segmento(registro(A));
		free(aux);
	}
	else {
		Quantum = 0;
		KernelMode = false;
		Execution_State = CPU_ABORT;
	}
}

void innn (void) {

	t_msg *msg = argv_message(NUMERIC_INPUT,1,PID);

	if(enviar_mensaje(Kernel,msg) == -1) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);

	if((msg = recibir_mensaje(Kernel)) == NULL || msg->header.id != REPLY_NUMERIC_INPUT) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	registro(A) = msg->argv[0];

	destroy_message(msg);
}

void innc (void) {

	t_msg *msg = argv_message(STRING_INPUT,2,PID,registro(B));

	if(enviar_mensaje(Kernel,msg) == -1) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);

	if((msg = recibir_mensaje(Kernel)) == NULL || msg->header.id != REPLY_STRING_INPUT) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	escribir_memoria(registro(A),msg->stream,registro(B));

	destroy_message(msg);
}

void outn (void) {

	t_msg *msg = argv_message(NUMERIC_OUTPUT,2,PID,registro(A));

	if(enviar_mensaje(Kernel,msg) == -1) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
}

void outc (void) {

	char *buffer = solicitar_memoria(registro(A),registro(B));

	if(Execution_State != CPU_ABORT) {

		t_msg *msg = string_message(STRING_OUTPUT,buffer,1,PID);

		if(enviar_mensaje(Kernel,msg) == -1) {
			puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
			exit(EXIT_FAILURE);
		}

		destroy_message(msg);
	}
}

void crea (void) {

	t_msg *msg = tcb_message(CPU_CREA,&Hilo,0);

	if(enviar_mensaje(Kernel,msg) == -1) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
}

void join (void) {

	t_msg *msg = argv_message(CPU_JOIN,2,Hilo.tid,registro(A));

	if(enviar_mensaje(Kernel,msg) == -1) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
}

void blok (void) {

	t_msg *msg = tcb_message(CPU_BLOCK,&Hilo,1,registro(B));

	if(enviar_mensaje(Kernel,msg) == -1) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
}

void wake (void) {

	t_msg *msg = argv_message(CPU_WAKE,1,registro(B));

	if(enviar_mensaje(Kernel,msg) == -1) {
		puts("ERROR: No se pudo solicitar el servicio requerido al Kernel.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
}

void inicializar_tabla_instrucciones(void) {

	alloc_segs = list_create();

	SetInstruccionesDeUsuario = dictionary_create();
	SetInstruccionesProtegidas = dictionary_create();

	dictionary_put(SetInstruccionesDeUsuario,"LOAD",load);
	dictionary_put(SetInstruccionesDeUsuario,"GETM",getm);
	dictionary_put(SetInstruccionesDeUsuario,"SETM",setm);
	dictionary_put(SetInstruccionesDeUsuario,"MOVR",movr);
	dictionary_put(SetInstruccionesDeUsuario,"ADDR",addr);
	dictionary_put(SetInstruccionesDeUsuario,"SUBR",subr);
	dictionary_put(SetInstruccionesDeUsuario,"MULR",mulr);
	dictionary_put(SetInstruccionesDeUsuario,"MODR",modr);
	dictionary_put(SetInstruccionesDeUsuario,"DIVR",divr);
	dictionary_put(SetInstruccionesDeUsuario,"INCR",incr);
	dictionary_put(SetInstruccionesDeUsuario,"DECR",decr);
	dictionary_put(SetInstruccionesDeUsuario,"COMP",comp);
	dictionary_put(SetInstruccionesDeUsuario,"CGEQ",cgeq);
	dictionary_put(SetInstruccionesDeUsuario,"CLEQ",cleq);
	dictionary_put(SetInstruccionesDeUsuario,"GOTO",eso_goto);
	dictionary_put(SetInstruccionesDeUsuario,"JMPZ",jmpz);
	dictionary_put(SetInstruccionesDeUsuario,"JPNZ",jpnz);
	dictionary_put(SetInstruccionesDeUsuario,"INTE",inte);
	dictionary_put(SetInstruccionesDeUsuario,"SHIF",shif);
	dictionary_put(SetInstruccionesDeUsuario,"NOPP",nopp);
	dictionary_put(SetInstruccionesDeUsuario,"PUSH",eso_push);
	dictionary_put(SetInstruccionesDeUsuario,"TAKE",take);
	dictionary_put(SetInstruccionesDeUsuario,"XXXX",xxxx);

	dictionary_put(SetInstruccionesProtegidas,"MALC",malc);
	dictionary_put(SetInstruccionesProtegidas,"FREE",eso_free);
	dictionary_put(SetInstruccionesProtegidas,"INNN",innn);
	dictionary_put(SetInstruccionesProtegidas,"INNC",innc);
	dictionary_put(SetInstruccionesProtegidas,"OUTN",outn);
	dictionary_put(SetInstruccionesProtegidas,"OUTC",outc);
	dictionary_put(SetInstruccionesProtegidas,"CREA",crea);
	dictionary_put(SetInstruccionesProtegidas,"JOIN",join);
	dictionary_put(SetInstruccionesProtegidas,"BLOK",blok);

}
