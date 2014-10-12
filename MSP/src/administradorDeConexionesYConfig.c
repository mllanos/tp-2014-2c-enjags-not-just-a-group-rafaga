/*
 * administradorDeConexiones.c
 *
 *  Created on: 09/10/2014
 *      Author: matias
 */

#include "administradorDeConexionesYConfig.h"

void cargarConfiguracion(char* path) {

	t_config* config = config_create(path);
	Puerto = config_get_int_value(config,"PUERTO");
	MaxMem = config_get_int_value(config,"CANTIDAD_MEMORIA")*K;
	MaxSwap = config_get_int_value(config,"CANTIDAD_SWAP")*M;
	AlgoritmoSustitucion = config_get_string_value(config,"SUST_PAGS");
	config_destroy(config);

}

void *atenderConsola(void* parametro) {

	return NULL;

}

void *atenderProceso(void* parametro) {

	t_msg *msg;
	int idSolicitud = 0,fd = *((int*)parametro);

	while(true) {
		msg = recibir_mensaje(fd);
		//recuperar_tipo_de_solicitud()
		switch(idSolicitud) {
		case WRITE_MEM:
			//recuperar_parametros()
			//ejecutar_rutina_correspondiente()
			break;
		case 1://CREATE_SEG:
			//recuperar_parametros()
			//ejecutar_rutina_correspondiente()
			break;
		case 2://DESTROY_SEG:
			//recuperar_parametros()
			//ejecutar_rutina_correspondiente()
			break;
		case 3://REQUEST_MEM:
			//recuperar_parametros()
			//ejecutar_rutina_correspondiente()
			break;
		}

		destroy_message(msg);
	}

	return NULL;	//para que no joda eclipse con el warning
}
