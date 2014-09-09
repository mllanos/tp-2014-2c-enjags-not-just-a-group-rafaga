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
#include "dummys.h"

int main(void) {

	if(conectarAKernel() == -1){
		log("error_conectar_kernel");
		mostrarPorPantalla("Hubo un error al conectar al Kernel. Abortado.");	//imprimir el mensaje en la consola donde se ejecuta el proceso, no la consola remota
		exit(EXIT_FAILURE);
	}

	if(conectarAMSP() == -1){
		log("error_conectar_msp");
		mostrarPorPantalla("Hubo un error al conectar a la MSP. Abortado.");
		exit(EXIT_FAILURE);
	}

	hilo_t hilo;				//TAD hilo (por ahi conviene hacerlo módulo, como LA pila de Solá). Contiene el TCB y el quantum correspondiente.
	int32_t A,B,C,D,E;			//registros de programación

	while(1){

		hilo_obtener(hilo);		//solicita un nuevo hilo para ejecutar (TCB y quantum) al Kernel, y lo carga en hilo.
		hilo_ejecutar(hilo);	//ejecuta las instrucciones del hilo mientras haya quantum.
		hilo_devolver(hilo);	//devuelve el hilo al kernel. Nota: como no necesita quantum restante, podría solo devolver el tcb.

	}

	return EXIT_SUCCESS;

}
