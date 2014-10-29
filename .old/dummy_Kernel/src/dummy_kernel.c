#include <stdio.h>
#include <stdlib.h>
#include <utiles/utiles.h>
#include <commons/string.h>
#include <pthread.h>

int msp;
t_hilo hilo;
void* atender_consola(void *parametro);

int main(void) {

	int i;
	t_msg *msg;
	pthread_t thread;
	uint16_t quantum = 4;

	hilo.cola = READY;
	hilo.kernel_mode = 1;
	hilo.pid = 74;
	hilo.puntero_instruccion = 0;
	for (i = 0; i < 5; ++i)
		hilo.registros[i] = 0;
	hilo.tid = 0;

	msp = client_socket("127.0.0.1",2233);

	int listener = server_socket(1122);

	int consola = accept_connection(listener);

	pthread_create(&thread,NULL,atender_consola,&consola);

	int cpu = accept_connection(listener);

	close(listener);

	while(1) {
	
		puts("Esperando solicitud de cpu\n");		
		
		msg = recibir_mensaje(cpu);
		switch(msg->header.id) {
			case FINISHED_THREAD:
				puts("tcb recibido\n");

				memcpy(&hilo,msg->stream,msg->header.length);

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

				puts("Finalizó la ejecución");
				return 0;
				break;
			case RETURN_TCB:
				puts("tcb recibido\n");

				memcpy(&hilo,msg->stream,msg->header.length);

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
				break;
			case CPU_TCB:
				destroy_message(msg);
				msg = tcb_message(NEXT_TCB,&hilo,1,quantum);
				enviar_mensaje(cpu,msg);	
				puts("TCB enviado\n\n");			
				break;		
			case STRING_OUTPUT:
			case NUMERIC_OUTPUT:
				enviar_mensaje(consola,msg);			
				break;	
			case STRING_INPUT:
				msg->argv = realloc(msg->argv,sizeof(int32_t)*2);
				msg->header.argc = 2;
				msg->argv[1] = cpu;
				enviar_mensaje(consola,msg);			
				break;	
			case NUMERIC_INPUT:
				msg->argv = realloc(msg->argv,sizeof(int32_t));
				msg->header.argc = 1;
				msg->argv[0] = cpu;
				enviar_mensaje(consola,msg);			
				break;	
		}

		destroy_message(msg);
	}

	return EXIT_SUCCESS;

}

void* atender_consola(void *parametro) {

	t_msg *msg,*msg2;
	int consola = *((int*) parametro);

	while(1) {
	
		puts("Esperando solicitud de consola\n");		
		
		msg = recibir_mensaje(consola);
		switch(msg->header.id) {
			case INIT_CONSOLE:
				msg2 = argv_message(CREATE_SEGMENT,2,74,msg->header.length);

				hilo.segmento_codigo_size = msg->header.length;

				enviar_mensaje(msp,msg2);

				destroy_message(msg2);

				msg2 = recibir_mensaje(msp);

				hilo.segmento_codigo = msg2->argv[0];

				destroy_message(msg2);



				msg2 = argv_message(CREATE_SEGMENT,2,74,2048);

				enviar_mensaje(msp,msg2);

				destroy_message(msg2);

				msg2 = recibir_mensaje(msp);

				hilo.cursor_stack = hilo.base_stack = msg2->argv[0];

				destroy_message(msg2);



				msg->header.id = WRITE_MEMORY;
				msg->header.argc = 2;
				msg->argv = malloc(sizeof(uint32_t)*2);
				msg->argv[0] = 74;
				msg->argv[1] = hilo.segmento_codigo;

				//write_file("besoDeConsola.bc",msg->stream,msg->header.length);

				enviar_mensaje(msp,msg);
				break;
			REPLY_NUMERIC_INPUT:
				msg2 = argv_message(REPLY_NUMERIC_INPUT,1,msg->argv[1]);
				enviar_mensaje(msg->argv[0],msg2);
				destroy_message(msg2);
				break;	
			REPLY_STRING_INPUT:
				msg2 = string_message(REPLY_STRING_INPUT,msg->stream,0);
				enviar_mensaje(msg->argv[0],msg2);
				destroy_message(msg2);	
				break;	
			default: 
				puts("ID inválido");
				break;	
		}

	destroy_message(msg);

	return NULL;
	}
}
