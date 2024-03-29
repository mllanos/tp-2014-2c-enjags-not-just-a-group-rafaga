#include "panel.h"
#include "kernel.h"
#include "cpu.h"

t_tipo_proceso proceso_tipo;

void inicializar_panel(t_tipo_proceso tipo_proceso, char* path){
	char* tipo_proceso_str;

	if (tipo_proceso == KERNEL)
		tipo_proceso_str = "kernel";
	else if (tipo_proceso == CPU)
		tipo_proceso_str = "cpu";
	else
		tipo_proceso_str = "?";

	proceso_tipo = tipo_proceso;

	char* logFile = string_duplicate(path);
	string_append(&logFile, tipo_proceso_str);
	string_append(&logFile, ".log");

	remove(logFile);
	logger = log_create(logFile, tipo_proceso_str, true, LOG_LEVEL_INFO);

	log_info(logger, "Inicializando panel para %s, en \"%s\"", tipo_proceso_str, logFile);

	free(logFile);

	kernel_cpus_conectadas = list_create();
	kernel_consolas_conectadas = list_create();
}


void finalizar_panel(void)
{
	if(proceso_tipo == KERNEL) {
		list_destroy_and_destroy_elements(kernel_cpus_conectadas, (void *) free);
		list_destroy_and_destroy_elements(kernel_consolas_conectadas, (void *) free);
	}

	log_destroy(logger);
}