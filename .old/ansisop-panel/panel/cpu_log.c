#include <stdio.h>
#include "cpu_log.h"

char* itoa(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

void ejecucion_hilo(t_hilo* hilo, uint32_t quantum) {

	printf("Comienza a ejecutar el hilo { PID: %d, TID: %d }", hilo->pid, hilo->tid);

	if (hilo->kernel_mode)
		printf(" en modo kernel");

	printf("\n");
}

void ejecucion_instruccion(char* mnemonico, t_list* parametros) {

	printf("Ejecutar instrucción %s; Parámetros: [", mnemonico);

	char *parametro;
	bool primero = true;
	while((parametro = (char*) list_remove(parametros,0)) != NULL) {
		if (!primero) printf(", ");
		printf("%s", parametro);
		primero = false;
	}

	printf("]\n");
}

void cambio_registros(t_registros_cpu registros) {

	printf("Registros: { A: %d, B: %d, C: %d, D: %d, E: %d, M: %u, P: %u, X: %u, S: %u, K: %u, I: %u }\n",
	registros.registros_programacion[0],
	registros.registros_programacion[1],
	registros.registros_programacion[2],
	registros.registros_programacion[3],
	registros.registros_programacion[4],
	registros.M, registros.P, registros.X, registros.S, registros.K, registros.I);
}

void fin_ejecucion() {
	printf("La CPU empieza a estar iddle\n");
}
