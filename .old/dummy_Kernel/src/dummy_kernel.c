#include <stdio.h>
#include <stdlib.h>
#include <utiles/utiles.h>
#include <commons/string.h>

int main(int argc,char **argv) {

	int i;
	t_hilo hilo;
	uint16_t quantum = 2;

	hilo.base_stack = 17632785;
	hilo.cola = READY;
	hilo.cursor_stack = 127;
	hilo.kernel_mode = 0;
	hilo.pid = 74;
	hilo.puntero_instruccion = 0;
	for (i = 0; i < 5; ++i)
		hilo.registros[i] = 0;
	hilo.segmento_codigo_size = 1024;
	hilo.tid = 0;

	int msp = client_socket("127.0.0.1",2233);

	t_msg *msg = argv_message(CREATE_SEGMENT,2,74,1024);

	enviar_mensaje(msp,msg);

	destroy_message(msg);

	msg = recibir_mensaje(msp);

	hilo.segmento_codigo = msg->argv[0];

	destroy_message(msg);

	msg = beso_message(WRITE_MEMORY,argv[1],2,74,hilo.segmento_codigo);

	enviar_mensaje(msp,msg);

	destroy_message(msg);

	int listener = server_socket(1122);

	int cpu = accept_connection(listener);

	close(listener);

	while(1) {
	
		puts("Esperando solicitud de cpu\n");		
		
		msg = recibir_mensaje(cpu);
		switch(msg->header.id) {
			case FINISHED_THREAD:
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
		}

		destroy_message(msg);
	}

	return EXIT_SUCCESS;

}
