/*
 * execution_unit.c
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#include "execution_unit.h"

void obtener_siguiente_hilo(void) {

	t_msg *msg = id_message(CPU_TCB);

	enviar_mensaje(Kernel,msg);

	msg = recibir_mensaje(Kernel);

	if(msg->header.id != NEXT_TCB)
		puts("No se pudo obtener el siguiente hilo a ejecutar");

	Quantum = msg->argv[0];
	Execution_State = CPU_TCB;
	memcpy(&Hilo,msg->stream,sizeof(t_hilo));

	destroy_message(msg);
}

void eu_cargar_registros(void) {

	int i;
	for (i = 0;i < 5; ++i)
		Registros.registros_programacion[i] = Hilo.registros[i];

	Registros.I = Hilo.pid;
	Registros.X = Hilo.base_stack;
	Registros.K = Hilo.kernel_mode;
	Registros.S = Hilo.cursor_stack;
	Registros.M = Hilo.segmento_codigo;
	Registros.P = Hilo.puntero_instruccion;
}

void eu_decode(char *operation_code) {

	Instruccion = dictionary_get(SetInstruccionesDeUsuario,operation_code);

	if (Registros.K && *Instruccion == NULL)
		Instruccion = dictionary_get(SetInstruccionesProtegidas,operation_code);

}

void eu_ejecutar(char *operation_code,uint32_t retardo) {

	msleep(retardo);

	--Quantum;
	Instruccion();

	//ejecucion_instruccion();
	cambio_registros(Registros);
}

void eu_actualizar_registros(void) {
	int i;
	for (i = 0;i < 5; ++i)
		Hilo.registros[i] = Registros.registros_programacion[i];

	Hilo.cursor_stack = Registros.S;
	Hilo.puntero_instruccion = Registros.P;
}

void devolver_hilo() {

	t_msg *msg = tcb_message(Execution_State, &Hilo, 0);

	enviar_mensaje(Kernel, msg);

	destroy_message(msg);
}

int fetch_operand(t_operandos tipo_operando) {

	uint8_t size;

	if(tipo_operando == REGISTRO)
		size = sizeof(char);
	else
		size = sizeof(uint32_t);

	return atoi(solicitar_memoria(program_counter,Instruction_size += size));
}

uint32_t crear_segmento(uint32_t size,t_msg_id *id) {

	uint32_t aux;

	t_msg *msg = argv_message(CREATE_SEGMENT,2,PID,size);

	enviar_mensaje(MSP,msg);

	destroy_message(msg);

	msg = recibir_mensaje(MSP);

	*id = msg->header.id;
	aux = (uint32_t) msg->argv[0];

	destroy_message(msg);

	return aux;
}

char* solicitar_memoria(uint32_t direccionLogica,uint32_t size) {

	char *buffer;

	t_msg *msg = argv_message(REQUEST_MEMORY,3,PID,direccionLogica,size);

	enviar_mensaje(MSP,msg);

	destroy_message(msg);

	msg = recibir_mensaje(MSP);

	if(msg->header.id != OK_REQUEST)
		puts("No se pudo acceder a la memoria");

	buffer = msg->stream;

	destroy_message(msg);

	return buffer;

}

t_msg_id escribir_memoria(uint32_t direccionLogica,char *bytesAEscribir,uint32_t size) {

	t_msg_id id;

	t_msg *msg = string_message(WRITE_MEMORY,bytesAEscribir,3,PID,direccionLogica,size);

	enviar_mensaje(MSP,msg);

	destroy_message(msg);

	free(bytesAEscribir);

	msg = recibir_mensaje(MSP);

	id = msg->header.id;

	destroy_message(msg);

	return id;
}
