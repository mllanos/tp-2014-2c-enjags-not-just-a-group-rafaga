/*
 * execution_unit.c
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#include "execution_unit.h"

void obtener_siguiente_hilo(void) {

	t_msg *msg = id_message(CPU_TCB);

	if(enviar_mensaje(Kernel,msg) == -1) {
		puts("ERROR: No se pudo obtener el siguiente hilo a ejecutar.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);

	if((msg = recibir_mensaje(Kernel)) == NULL || msg->header.id != NEXT_TCB) {
		puts("ERROR: No se pudo obtener el siguiente hilo a ejecutar.");
		exit(EXIT_FAILURE);
	}

	Quantum = msg->argv[0];
	Execution_State = RETURN_TCB;
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

	if (Registros.K && Instruccion == NULL)
		Instruccion = dictionary_get(SetInstruccionesProtegidas,operation_code);

	if(Instruccion == NULL) {
		puts("ERROR: Instrucción inválida.");
		exit(EXIT_FAILURE);
	}

	Parametros = list_create();

}

void eu_ejecutar(char *operation_code,uint32_t retardo) {

	msleep(retardo);

	--Quantum;
	Instruccion();

	ejecucion_instruccion(operation_code,Parametros);
	cambio_registros(Registros);
	list_destroy(Parametros);

	free(operation_code);
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

	if(enviar_mensaje(Kernel, msg) == -1) {
		puts("ERROR: No se pudo enviar el TCB del hilo en ejecución.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
}

int fetch_operand(t_operandos tipo_operando) {

	int32_t aux;
	uint8_t size;
	char *buffer;
	char *parametro;

	if(tipo_operando == REGISTRO) {
		size = sizeof(char);
		buffer = solicitar_memoria(program_counter + Instruction_size,size);
		parametro = malloc(2);
		*parametro = aux = *buffer;
		parametro[1] = '\0';
		list_add(Parametros,parametro);
	}
	else {
		size = sizeof(uint32_t);
		buffer = solicitar_memoria(program_counter + Instruction_size,size);

		buffer[0];
		buffer[1];
		buffer[2];
		buffer[3];

		memcpy(&aux,buffer,size);
		parametro = string_itoa(aux);
		list_add(Parametros,parametro);
	}

	Instruction_size += size;

	free(buffer);

	return aux;
}

uint32_t crear_segmento(uint32_t size,t_msg_id *id) {

	uint32_t aux;

	t_msg *msg = argv_message(CREATE_SEGMENT,2,PID,size);

	if(enviar_mensaje(MSP,msg) == -1) {
		puts("ERROR: No se pudo crear el segmento solicitado.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);

	if((msg = recibir_mensaje(MSP)) == NULL || msg->header.id != OK_CREATE) {
		puts("ERROR: No se pudo crear el segmento solicitado.");
		exit(EXIT_FAILURE);
	}

	*id = msg->header.id;
	aux = (uint32_t) msg->argv[0];

	destroy_message(msg);

	return aux;
}

char* solicitar_memoria(uint32_t direccionLogica,uint32_t size) {

	char *buffer = malloc(size);

	t_msg *msg = argv_message(REQUEST_MEMORY,3,PID,direccionLogica,size);

	if(enviar_mensaje(MSP,msg) == -1) {
		puts("ERROR: No se pudo acceder a la memoria.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);

	if((msg = recibir_mensaje(MSP)) == NULL || msg->header.id != OK_REQUEST) {
		puts("ERROR: No se pudo acceder a la memoria.");
		exit(EXIT_FAILURE);
	}

	memcpy(buffer,msg->stream,size);

	destroy_message(msg);

	return buffer;

}

t_msg_id escribir_memoria(uint32_t direccionLogica,char *bytesAEscribir,uint32_t size) {

	t_msg_id id;

	t_msg *msg = string_message(WRITE_MEMORY,bytesAEscribir,2,PID,direccionLogica);

	msg->header.length = size;

	if(enviar_mensaje(MSP,msg) == -1) {
		puts("ERROR: No se pudo escribir en la memoria.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
	free(bytesAEscribir);

	if((msg = recibir_mensaje(MSP)) == NULL || msg->header.id != OK_WRITE) {
		puts("ERROR: No se pudo escribir en la memoria.");
		exit(EXIT_FAILURE);
	}

	id = msg->header.id;

	destroy_message(msg);

	return id;
}
