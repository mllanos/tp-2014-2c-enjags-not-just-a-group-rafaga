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

void obtener_siguiente_hilo (void) {
//falta bucle hasta comprobar el id
	t_msg *buffer = recibir_mensaje(kernel);
	memcpy(&quantum,buffer->stream,sizeof quantum);
	memcpy(&hilo.pid,buffer->stream + 2,4);
	//memcpy(&hilo,buffer->stream + sizeof quantum,buffer->header.length - sizeof quantum);
	deserializar_tcb(&hilo,buffer->stream + 2);

	int i;
	printf("Quantum Valor:		%8d\n", quantum);
	printf("Registro PID Valor:		%8d\n", hilo.pid);
	printf("Registro TID Valor:		%8d\n", hilo.tid);
	printf("Registro KM Valor:		%8d\n", hilo.kernel_mode);
	printf("Registro CS Valor:		%8d\n", hilo.segmento_codigo);
	printf("Registro CS_Size Valor:		%8d\n", hilo.segmento_codigo_size);
	printf("Registro IP Valor:		%8d\n", hilo.puntero_instruccion);
	printf("Registro Stack Valor:		%8d\n", hilo.base_stack);
	printf("Registro Stack_Size Valor:	%8d\n", hilo.cursor_stack);
	for(i = 0;i < 5; ++i)
		printf("Registro %c. Valor:		%8d\n",('A'+i), hilo.registros[i]);
	printf("Registro COLA:			%8d\n", hilo.cola);
	puts("\n");

	destroy_message(buffer);
}

void avanzar_puntero_instruccion(size_t desplazamiento){

	registros.P += instruccion_size;

}

void eu_cargar_registros(void){

	int i;
	for(i = 0;i < 5; ++i)
		registros.registros_programacion[i] = hilo.registros[i];

	registros.I = hilo.pid;
	registros.X = hilo.base_stack;
	registros.K = hilo.kernel_mode;
	registros.S = hilo.cursor_stack;
	registros.M = hilo.segmento_codigo;
	registros.P = hilo.puntero_instruccion;

}

void eu_actualizar_registros(void){

	int i;
	for(i = 0;i < 5; ++i)
		hilo.registros[i] = registros.registros_programacion[i];

	hilo.cursor_stack = registros.S;
	hilo.puntero_instruccion = registros.P;

}

void eu_fetch_instruccion(void){

	instruccion_size = OPERATION_CODE_SIZE;
	t_msg *new_msg = msp_solicitar_memoria(registros.I,registros.M + registros.P,OPERATION_CODE_SIZE,OC_REQUEST);
	//validar que el ID sea NEXT_OC
	printf("Recibi de MSP: ID %d - Tamanio %d - OC %s\n",new_msg->header.id,new_msg->header.length,new_msg->stream);
	fflush(stdout);
	memcpy(oc_instruccion,new_msg->stream,OPERATION_CODE_SIZE);
	destroy_message(new_msg);
	//fread(oc_instruccion,instruccion_size,1,tcb);
}

void eu_decode(void){

	if(registros.K == 0)
		fin_tabla = 22;
	else
		fin_tabla = 32;

	for(cursor_tabla = 0;cursor_tabla <= fin_tabla && strcmp(oc_instruccion,tabla_instrucciones[cursor_tabla].mnemonico);++cursor_tabla )
		;

	//if(cursor_tabla > fin_tabla)
		;//hacer algo con las instrucciones inválidas (no existe o no tiene privilegios para ejecutarla)

}

void eu_ejecutar(int retardo){

	msleep(retardo);
	tabla_instrucciones[cursor_tabla].rutina();
	--quantum;

}

int fetch_operand(t_operandos tipo_operando){

	int arg = 0;
	uint32_t size;

	if(tipo_operando == REGISTRO)
		size = sizeof(char);
	else
		size = sizeof(int32_t);

	t_msg *new_msg = msp_solicitar_memoria(registros.I,registros.M + registros.P,size,ARG_REQUEST);
	//validar el ID
	printf("Recibi de MSP: ID %d - Tamanio %d - ARG %d\n",new_msg->header.id,new_msg->header.length,(int)*(new_msg->stream));
	fflush(stdout);
	memcpy(&arg,new_msg->stream,size);
	instruccion_size += size;
	destroy_message(new_msg);

	return arg;

}

void devolver_hilo(void) {

	t_msg *buffer = crear_mensaje(CPU_TCB,(char*) &hilo,56);
	enviar_mensaje(kernel,buffer);
	free(buffer);

}

t_msg* msp_solicitar_memoria(uint32_t pid,uint32_t direccion_logica,uint32_t size, t_msg_id id) {

	char *stream = malloc(2*sizeof pid + sizeof size);
	memcpy(stream,&pid,sizeof pid);
	memcpy(stream + sizeof pid,&direccion_logica,sizeof direccion_logica);
	memcpy(stream + 2*sizeof pid,&size,sizeof size);

	t_msg *new_msg = crear_mensaje(id,stream,2*sizeof pid + sizeof size);

	enviar_mensaje(msp,new_msg);
	destroy_message(new_msg);

	return recibir_mensaje(msp);

}
