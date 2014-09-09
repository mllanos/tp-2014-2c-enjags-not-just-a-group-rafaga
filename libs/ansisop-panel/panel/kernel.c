#include "kernel.h"
#include <stdio.h>

void conexion_cpu(uint32_t id) {
	printf("Se conectó la CPU %d\n", id);
}
void desconexion_cpu(uint32_t id) {
	printf("Se desconectó la CPU %d\n", id);
}

void conexion_consola(uint32_t id) {
	printf("Se conectó la Consola %d\n", id);
}

void desconexion_consola(uint32_t id) {
	printf("Se desconectó la Consola %d\n", id);
}

void hilos(t_list* hilos) {
	printf("Hilos ejecutando: [");

	bool primero = true;
	void _imprimirPid(t_hilo* hilo) {
		if (!primero) printf(", ");
		printf("{ PID: %d, TID: %d }", hilo->pid, hilo->tid);
		primero = false;
	}
	list_iterate(hilos, (void*) _imprimirPid);

	printf("]\n");
}

void instruccion_protegida(char* mnemonico, t_hilo* hilo) {
	printf("El hilo { PID: %d, TID: %d } ejecutó la instrucción %s\n", hilo->pid, hilo->tid, mnemonico);
}
