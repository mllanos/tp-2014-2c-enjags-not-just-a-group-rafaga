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
#include "funciones.h"
#include "dummys.h"
#define OPERATION_CODE_SIZE 4;

int main(void) {

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

	size_t instruccion_size = 4;

	while(1){

		//obtener_siguiente_hilo();		//solicita un nuevo hilo para ejecutar (TCB y quantum) al Kernel.

		while(quantum || hilo.kernel_mode){

			cargar_registros();
			//instruccion_size = obtener_siguiente_instruccion();
			//ejecutar_instruccion();
			actualizar_registros_tcb();
			avanzar_puntero_instruccion(instruccion_size);

		}

		//devolver_hilo();						//devuelve el hilo al kernel.

	}

	return EXIT_SUCCESS;

}


