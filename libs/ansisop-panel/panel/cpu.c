#include "cpu.h"
#include <stdio.h>

void comienzo_ejecucion(t_hilo* hilo, uint32_t quantum) {
	printf("Comienza a ejecutar el hilo { PID: %d, TID: %d }", hilo->pid, hilo->tid);
	if (hilo->kernel_mode) printf(" en modo kernel");
	printf("\n");
}

void fin_ejecucion() {
	printf("La CPU empieza a estar iddle\n");
}

void ejecucion_instruccion(char* mnemonico, t_list* parametros) {
	printf("Ejecutar instrucción %s; Parámetros: [", mnemonico);

	bool primero = true;
	void _imprimirParametro(char* parametro) {
		if (!primero) printf(", ");
		printf("%s", parametro);
		primero = false;
	}
	list_iterate(parametros, (void*) _imprimirParametro);

	printf("]\n");
}

void cambio_registros(t_registros_cpu registros) {
	printf("Registros: { A: %d, B: %d, C: %d, D: %d, E: %d, M: %d, P: %d, S: %d, K: %d, I: %d }\n",
		registros.registros_programacion[0],
		registros.registros_programacion[1],
		registros.registros_programacion[2],
		registros.registros_programacion[3],
		registros.registros_programacion[4],
		registros.M, registros.P, registros.S, registros.K, registros.I
	);
}

//-------------------------------------------------
//Retrocompatibilidad con el ejemplo del enunciado:
void ejecucion_hilo(t_hilo* hilo, uint32_t quantum) {
	comienzo_ejecucion(hilo, quantum);
}
