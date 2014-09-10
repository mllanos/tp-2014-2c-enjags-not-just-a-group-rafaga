/*
 * funciones.h
 *
 *  Created on: 10/09/2014
 *      Author: matias
 */

uint32_t quantum;
t_instruccion instruccion;
t_registros_cpu registros;
t_hilo hilo;

void avanzar_puntero_instruccion(size_t desplazamiento){

	hilo.puntero_instruccion += desplazamiento;

}

void actualizar_registros_tcb(void){

	int i;
	for(i = 0;i < 5; ++i)
		hilo.registros[i] = registros.registros_programacion[i];

}

void cargar_registros(void){

	int i;
	for(i = 0;i < 5; ++i)
		registros.registros_programacion[i] = hilo.registros[i];

}

void ejecutar_instruccion(void){



}
