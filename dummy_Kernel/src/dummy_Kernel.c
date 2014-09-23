#include <utiles/utiles.h>
#include <stdio.h>
#include <stdlib.h>
#define PORT 1122

int main(void) {

	int i;
	t_hilo hilo,hilo2;
	uint16_t quantum = 2;

	hilo.base_stack = 17632785;
	hilo.cola = READY;
	hilo.cursor_stack = 127;
	hilo.kernel_mode = 0;
	hilo.pid = 1147;
	hilo.puntero_instruccion = 0;
	for (i = 0; i < 5; ++i)
		hilo.registros[i] = 0;
	hilo.segmento_codigo = 38155741;
	hilo.segmento_codigo_size = 1024;
	hilo.tid = 0;

	char* stream = serializar_tcb(&hilo,quantum);
	deserializar_tcb(&hilo2,stream + 2);

	int listener = server_socket(PORT);
	int cpu = accept_connection(listener);
	close(listener);

	t_msg *next_hilo = new_message(NEXT_TRHEAD,stream,58);
	enviar_mensaje(cpu,next_hilo);
	puts("TCB enviado (primera vez)\n\n");
	printf("%d\n", next_hilo->header.length);
	puts("\n");
	destroy_message(next_hilo);

	while(1) {

		puts("Esperando tcb de cpu\n");
		t_msg *tcb_cpu = recibir_mensaje(cpu);
		puts("tcb recibido\n");
		deserializar_tcb(&hilo,tcb_cpu->stream);

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

		stream = serializar_tcb(&hilo,quantum);

		next_hilo = new_message(NEXT_TRHEAD,stream,58);

		enviar_mensaje(cpu,next_hilo);
		puts("TCB enviado\n\n");
		destroy_message(next_hilo);

	}

	close(cpu);
/*
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
*/
	return 0;

}
