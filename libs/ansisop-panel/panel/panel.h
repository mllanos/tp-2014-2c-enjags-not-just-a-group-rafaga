#ifndef PANEL_H_
#define PANEL_H_

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdint.h>
	#include <stdbool.h>
	#include <string.h>
	#include "commons/collections/list.h"
	#include "commons/string.h"
	#include "commons/log.h"

	typedef enum { KERNEL, CPU } t_tipo_proceso;
	typedef enum { NEW, READY, EXEC, BLOCK, EXIT } t_cola;

	typedef struct {
		uint32_t pid;
		uint32_t tid;
		bool kernel_mode;
		uint32_t segmento_codigo;
		uint32_t segmento_codigo_size;
		uint32_t puntero_instruccion;
		uint32_t base_stack;
		uint32_t cursor_stack;
		int32_t registros[5];
		t_cola cola;
	} t_hilo;

	t_log *logger;

	/*
	 * Las funciones declaradas en los headers panel.h, cpu.h,
	 * y kernel.h deben ser invocadas a modo de notificación de
	 * los eventos que representan.
	 */

	/**
	 * Debe invocarse luego de inicializar el proceso en cuestión (CPU o Kernel).
	 * Por ejemplo: inicializar_panel(CPU, "/home/utnso/panel");
	 *
	 * @param  tipo_proceso Indica si es un CPU o un Kernel.
	 * @param  path  Es la ruta absoluta a un directorio donde el panel pueda
	 *               generar archivos de log libremente. El directorio debe existir.
	 **/
	void inicializar_panel(t_tipo_proceso tipo_proceso, char* path);

#endif
