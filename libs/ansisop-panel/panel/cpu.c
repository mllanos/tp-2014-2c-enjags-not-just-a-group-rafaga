#include "cpu.h"
#include <stdio.h>

#define comienzo_ejecucion ejecucion_hilo

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

	char* mensaje = string_new();

	string_append_with_format(&mensaje, "Comienza a ejecutar el hilo { PID: %d, TID: %d }", hilo->pid, hilo->tid);

	if (hilo->kernel_mode)
		string_append(&mensaje, " en modo kernel");

	log_info(logger, mensaje);

	free(mensaje);

}

void ejecucion_instruccion(char* mnemonico, t_list* parametros) {

	char* mensaje = string_new();
	string_append_with_format(&mensaje, "Instrucción %s [", mnemonico);

	bool primero = true;

	void _imprimirParametro(char* parametro) {

		if (!primero) string_append(&mensaje, ", ");
			string_append_with_format(&mensaje, "%s", parametro);

		primero = false;
	}

	list_iterate(parametros, (void*) _imprimirParametro);
	string_append(&mensaje, "]");

	log_info(logger, mensaje);

	free(mensaje);

}

void cambio_registros(t_registros_cpu registros) {

	printf("Registros: { A: %d, B: %d, C: %d, D: %d, E: %d, M: %d, P: %d, X: %d, S: %d, K: %d, I: %d }",
			registros.registros_programacion[0],
			registros.registros_programacion[1],
			registros.registros_programacion[2],
			registros.registros_programacion[3],
			registros.registros_programacion[4],
			registros.M, registros.P, registros.X, registros.S, registros.K, registros.I
	);

}

void fin_ejecucion() {

	log_info(logger, "La CPU empieza a estar iddle");
}
