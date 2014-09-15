#ifndef PANEL_H_
#define PANEL_H_

	#include <stdint.h>
	#include <stdbool.h>
	#include "collections/list.h"

	typedef enum { KERNEL, CPU } t_tipo_proceso;

	/*Loggeo de eventos CPU*/
	void comienzo_ejecucion(t_hilo* hilo, uint32_t quantum);
	void fin_ejecucion();
	void ejecucion_instruccion(char* mnemonico, t_list* parametros);
	void cambio_registros(t_registros_cpu registros);
	void ejecucion_hilo(t_hilo* hilo, uint32_t quantum);
	/*FIN_Loggeo de eventos CPU*/

	/*Loggeo de eventos Kernel*/
	void conexion_cpu(uint32_t id);
	void desconexion_cpu(uint32_t id);
	void conexion_consola(uint32_t id);
	void desconexion_consola(uint32_t id);
	void instruccion_protegida(char* mnemonico, t_hilo* hilo);
	void inicializar_panel(t_tipo_proceso tipo_proceso, char* path);
	/*FIN_Loggeo de eventos Kernel*/

#endif
