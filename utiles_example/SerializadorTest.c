#include <utiles/utiles.h>
#include <stdio.h>
int main(void) {

	int i;
	t_hilo hilo,hilo2;
	uint16_t quantum = 27,quantum2 = 0;

	hilo.base_stack = 17632785;
	hilo.cola = READY;
	hilo.cursor_stack = 127;
	hilo.kernel_mode = 0;
	hilo.pid = 1147;
	hilo.puntero_instruccion = 2;
	for (i = 0; i < 5; ++i)
		hilo.registros[i] = 13 + i;
	hilo.segmento_codigo = 38155741;
	hilo.segmento_codigo_size = 1024;
	hilo.tid = 0;

	char* stream = serializar_tcb(&hilo,quantum);

	quantum2 = (uint16_t) *stream;
	deserializar_tcb(&hilo2,(stream + 2));

	printf("Quantum Valor:			%8d\n", quantum2);
	printf("Registro PID Valor:		%8d\n", hilo2.pid);
	printf("Registro TID Valor:		%8d\n", hilo2.tid);
	printf("Registro KM Valor:		%8d\n", hilo2.kernel_mode);
	printf("Registro CS Valor:		%8d\n", hilo2.segmento_codigo);
	printf("Registro CS_Size Valor:		%8d\n", hilo2.segmento_codigo_size);
	printf("Registro IP Valor:		%8d\n", hilo2.puntero_instruccion);
	printf("Registro Stack Valor:		%8d\n", hilo2.base_stack);
	printf("Registro Stack_Size Valor:	%8d\n", hilo2.cursor_stack);
	for(i = 0;i < 5; ++i)
		printf("Registro %c. Valor:		%8d\n",('A'+i), hilo2.registros[i]);
	printf("Registro COLA:			%8d\n", hilo2.cola);
	puts("\n");

	return 0;

}

void more_stuff(uint32_t value) {             // Example value: 0x01020304
    uint32_t byte1 = (value >> 24);           // 0x01020304 >> 24 is 0x01 so
                                              // no masking is necessary
    uint32_t byte2 = (value >> 16) & 0xff;    // 0x01020304 >> 16 is 0x0102 so
                                              // we must mask to get 0x02
    uint32_t byte3 = (value >> 8)  & 0xff;    // 0x01020304 >> 8 is 0x010203 so
                                              // we must mask to get 0x03
    uint32_t byte4 = value & 0xff;            // here we only mask, no shifting
                                              // is necessary
    ...
}
