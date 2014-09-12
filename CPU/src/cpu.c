/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "execution_unit.h"
#include "dummys.h"

t_hilo hilo;
uint32_t quantum;
size_t instruccion_size;

int main(void) {

	//Levantar archivo de configuracion
	t_config* config = config_create(PATH_ARCHIVO_CONF);
	int puerto_kernel = config_get_int_value(config,"PUERTO_KERNEL");
	char *direccionIP_kernel = config_get_string_value(config,"IP_KERNEL");
	int puerto_msp = config_get_int_value(config,"PUERTO_MSP");
	char *direccionIP_msp = config_get_string_value(config,"IP_MSP");
	int retardo = config_get_int_value(config,"RETARDO");
	//Fin levantar archivo de configuracion

	if(conectar_a_kernel() == -1){
		//log("error_conectar_kernel");
		puts("Hubo un error al conectar al Kernel. Abortado.");	//imprimir el mensaje en la consola donde se ejecuta el proceso, no la consola remota
		exit(EXIT_FAILURE);
	}

	if(conectar_a_msp() == -1){
		//log("error_conectar_msp");
		puts("Hubo un error al conectar a la MSP. Abortado.");
		exit(EXIT_FAILURE);
	}

	while(1){

		//obtener_siguiente_hilo();		//solicita un nuevo hilo para ejecutar (TCB y quantum) al Kernel.
		//aca va el delay con el retardo?
		while(quantum || hilo.kernel_mode){

			eu_cargar_registros();
			eu_fetch_instruccion();
			eu_decode();
			eu_ejecutar();
			avanzar_puntero_instruccion(instruccion_size);
			eu_actualizar_registros();

		}

		//devolver_hilo();						//devuelve el hilo al kernel.

	}

	return EXIT_SUCCESS;

}


