/*
 * execution_unit.c
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

#include "execution_unit.h"

/*Variables Locales*/
static t_list *parametros;
static char oc_instruccion[5];											//operation code
static int cursor_tabla,fin_tabla;
/*FIN_Variables Locales*/

void obtener_siguiente_hilo (void) {

	t_msg *buffer;
	parametros = list_create();

	do {
	buffer = recibir_mensaje(kernel);
	} while(buffer->header.id != NEXT_THREAD);

	memcpy(&quantum,buffer->stream,sizeof quantum);
	deserializar_tcb(&hilo,buffer->stream + sizeof quantum);
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
	//if(new_msg->header.id != NEXT_ARG)
		;//abortar la ejecucion?
	memcpy(oc_instruccion,new_msg->stream,OPERATION_CODE_SIZE);
	destroy_message(new_msg);

	list_destroy(parametros);
	parametros = list_create();
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
	ejecucion_instruccion(tabla_instrucciones[cursor_tabla].mnemonico, parametros);
	cambio_registros(registros);
	printf("\n\nREGISTRO A: %X\n\n",registros.registros_programacion[A]);
	fflush(stdout);

}

int fetch_operand(t_operandos tipo_operando){

	int arg = 0;
	uint32_t size;

	if(tipo_operando == REGISTRO)
		size = sizeof(char);
	else
		size = sizeof(int32_t);

	t_msg *new_msg = msp_solicitar_memoria(registros.I,registros.M + registros.P,size,ARG_REQUEST);
	//if(new_msg->header.id != NEXT_OC)
			;//abortar la ejecucion?
	memcpy(&arg,new_msg->stream,size);
	instruccion_size += size;
	destroy_message(new_msg);

	list_add(parametros,(void*)&arg);

	return arg;

}

void devolver_hilo(void) {

	t_msg *buffer = crear_mensaje(CPU_TCB,(char*) &hilo,sizeof(t_hilo));
	enviar_mensaje(kernel,buffer);
	free(buffer);

}

t_msg* msp_solicitar_memoria(uint32_t pid,uint32_t direccion_logica,uint32_t size, t_msg_id id) {

	int stream_size = 2*REG_SIZE + sizeof size;
	char *stream = malloc(stream_size);
	memcpy(stream,&pid,REG_SIZE);
	memcpy(stream + REG_SIZE,&direccion_logica,REG_SIZE);
	memcpy(stream + 2*REG_SIZE,&size,sizeof size);

	t_msg *new_msg = crear_mensaje(id,stream,stream_size);

	enviar_mensaje(msp,new_msg);
	destroy_message(new_msg);

	return recibir_mensaje(msp);

}

t_msg* msp_escribir_memoria(uint32_t pid,uint32_t direccion_logica,void *bytes_a_escribir,uint32_t size) {

	char *stream = malloc(2*REG_SIZE + size);
	memcpy(stream,&pid,REG_SIZE);
	memcpy(stream + REG_SIZE,&direccion_logica,REG_SIZE);
	memcpy(stream + 2*REG_SIZE,&bytes_a_escribir,size);

	t_msg *new_msg = crear_mensaje(WRITE_MEM,stream,size);

	enviar_mensaje(msp,new_msg);
	destroy_message(new_msg);

	return recibir_mensaje(msp);

}

void servicio_kernel() {



}
