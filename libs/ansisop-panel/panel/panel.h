#ifndef PANEL_H_
#define PANEL_H_

#include <stdint.h>
#include <stdbool.h>
#include <commons/collections/list.h>

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
} __attribute__ ((__packed__)) t_hilo;


	/*Loggeo de eventos Kernel*/
void conexion_cpu(uint32_t id);
void desconexion_cpu(uint32_t id);
void conexion_consola(uint32_t id);
void desconexion_consola(uint32_t id);
void instruccion_protegida(char* mnemonico, t_hilo* hilo);
void inicializar_panel(t_tipo_proceso tipo_proceso, char* path);
	/*FIN_Loggeo de eventos Kernel*/

#endif
