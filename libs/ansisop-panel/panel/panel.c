#include "panel.h"
#include "kernel.h"
#include "cpu.h"

void inicializar_panel(t_tipo_proceso tipo_proceso, char* path){
	char* tipo_proceso_str;

	if (tipo_proceso == KERNEL)
		tipo_proceso_str = "kernel";
	else if (tipo_proceso == CPU)
		tipo_proceso_str = "cpu";
	else
		tipo_proceso_str = "?";

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
