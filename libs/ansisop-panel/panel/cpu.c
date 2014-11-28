#include "cpu.h"
#include <stdio.h>

void comienzo_ejecucion(t_hilo* hilo, uint32_t quantum) {
	char* mensaje = string_new();

	string_append_with_format(&mensaje, "Ejecuta hilo { PID: %d, TID: %d }", hilo->pid, hilo->tid);
	if (hilo->kernel_mode) string_append(&mensaje, " en modo kernel");

	log_info(logger, mensaje);
	free(mensaje);
}

void fin_ejecucion() {
	log_info(logger, "Empieza a estar idle");
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
	log_info(logger, "Registros: { A: %d, B: %d, C: %d, D: %d, E: %d, M: %d, P: %d, S: %d, K: %d, I: %d }",
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
