#include "panel.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void inicializar_panel(t_tipo_proceso tipo_proceso, char* path){
	char* tipo_proceso_str;

	if (tipo_proceso == KERNEL)
		tipo_proceso_str = strdup("Kernel");
	else if (tipo_proceso == CPU)
		tipo_proceso_str = strdup("CPU");
	else
		tipo_proceso_str = strdup("?");


	printf("Inicializando panel para: %s, en ruta \"%s\"\n.", tipo_proceso_str, path);

	free(tipo_proceso_str);
}
