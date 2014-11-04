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
	Registros.S = Hilo.cursor_stack;
	Registros.M = Hilo.segmento_codigo;
	Registros.P = Hilo.puntero_instruccion;
	KernelMode = (Registros.K = Hilo.kernel_mode);
}

void eu_decode(char *operation_code) {

	if((Instruccion = dictionary_get(SetInstruccionesDeUsuario,operation_code)) == NULL)
		if(!Registros.K || (Instruccion = dictionary_get(SetInstruccionesProtegidas,operation_code)) == NULL) {
			puts("ERROR: Instrucción inválida.");
			exit(EXIT_FAILURE);
		}

	Parametros = list_create();
}

void eu_ejecutar(char *operation_code,uint32_t retardo) {

	msleep(retardo);

	--Quantum;
	Instruccion();

	ejecucion_instruccion(operation_code,Parametros);	/* LOG */
	cambio_registros(Registros);						/* LOG */
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

	t_msg *msg = Kernel_Msg == NULL ? tcb_message(Execution_State, &Hilo, 0) : Kernel_Msg;

	if(enviar_mensaje(Kernel, msg) == -1) {
		puts("ERROR: No se pudo enviar el TCB del hilo en ejecución al Kernel.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
	Kernel_Msg = NULL;
}

int fetch_operand(t_operandos tipo_operando) {	//se puede mejorar con un Union

	uint8_t size;
	char *buffer;
	char *parametro;
	int32_t aux = 'A';

	size = tipo_operando == REGISTRO ? sizeof(char) : sizeof(uint32_t);
	buffer = solicitar_memoria(program_counter + Instruction_size,size);
	Instruction_size += size;

	if(buffer != NULL) {
		if(tipo_operando == REGISTRO) {
			parametro = malloc(2);
			*parametro = aux = *buffer;
			parametro[1] = '\0';
			list_add(Parametros,parametro);
		}
		else {
			memcpy(&aux,buffer,size);
			parametro = string_itoa(aux);
			list_add(Parametros,parametro);
		}

		free(buffer);
	}

	return aux;
}

uint32_t crear_segmento(uint32_t size) {

	uint32_t aux;

	t_msg *msg = argv_message(CREATE_SEGMENT,2,PID,size);

	if(enviar_mensaje(MSP,msg) == -1) {
		puts("ERROR: Se ha perdido la conexión con la MSP.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);

	if((msg = recibir_mensaje(MSP)) == NULL) {
		puts("ERROR: Se ha perdido la conexión con la MSP.");
		exit(EXIT_FAILURE);
	}

	if(msg->header.id == OK_CREATE)
		aux = (uint32_t) msg->argv[0];
	else {
		Quantum = 0;
		KernelMode = false;
		Execution_State = CPU_ABORT;
	}

	destroy_message(msg);

	return aux;
}

char* solicitar_memoria(uint32_t direccionLogica,uint32_t size) {

	char *buffer = malloc(size);

	t_msg *msg = argv_message(REQUEST_MEMORY,3,PID,direccionLogica,size);

	if(enviar_mensaje(MSP,msg) == -1) {
		puts("ERROR: Se ha perdido la conexión con la MSP.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);

	if((msg = recibir_mensaje(MSP)) == NULL) {
		puts("ERROR: Se ha perdido la conexión con la MSP.");
		exit(EXIT_FAILURE);
	}

	if(msg->header.id == OK_REQUEST)
		memcpy(buffer,msg->stream,size);
	else {
		free(buffer);
		buffer = NULL;
		Quantum = 0;
		KernelMode = false;
		Execution_State = CPU_ABORT;
	}

	destroy_message(msg);

	return buffer;

}

void escribir_memoria(uint32_t direccionLogica,char *bytesAEscribir,uint32_t size) {

	t_msg *msg = string_message(WRITE_MEMORY,bytesAEscribir,2,PID,direccionLogica);

	msg->header.length = size;

	if(enviar_mensaje(MSP,msg) == -1) {
		puts("ERROR: Se ha perdido la conexión con la MSP.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);
	free(bytesAEscribir);

	if((msg = recibir_mensaje(MSP)) == NULL) {
		puts("ERROR: Se ha perdido la conexión con la MSP.");
		exit(EXIT_FAILURE);
	}

	if(msg->header.id != OK_WRITE) {
		Quantum = 0;
		KernelMode = false;
		Execution_State = CPU_ABORT;
	}

	destroy_message(msg);

}

void destruir_segmento(uint32_t baseSegmento) {

	t_msg *msg = argv_message(DESTROY_SEGMENT,2,PID,baseSegmento);

	if(enviar_mensaje(MSP,msg) == -1) {
		puts("ERROR: Se ha perdido la conexión con la MSP.");
		exit(EXIT_FAILURE);
	}

	destroy_message(msg);

	if((msg = recibir_mensaje(MSP)) == NULL) {
		puts("ERROR: Se ha perdido la conexión con la MSP.");
		exit(EXIT_FAILURE);
	}

	if(msg->header.id != OK_DESTROY) {
		Quantum = 0;
		KernelMode = false;
		Execution_State = CPU_ABORT;
	}

	destroy_message(msg);

}
