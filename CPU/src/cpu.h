/*
 * cpu.h
 *
 *  Created on: 05/09/2014
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <utiles/utiles.h>
#include <panel/panel.h>
#include "execution_unit.h"

/*dummy*/
void imprimir_tcb(void);
int conectar_a_kernel(char *direccionIP,uint16_t puerto);
int conectar_a_msp(char *direccionIP,uint16_t puerto);
/*dummys*/

/* Logs */

/**
	 * Debe invocarse cada vez que un hilo ingrese a la CPU.
	 *
	 * @param  hilo  Estructura conteniendo todos los campos del TCB del hilo.
	 * @param  quantum  Tamaño del Quantum.
	 */
	void comienzo_ejecucion(t_hilo* hilo, uint32_t quantum);

	/**
	 * Debe invocarse cada vez que un hilo salga de la CPU.
	 */
	void fin_ejecucion();

	/**
	 * Debe invocarse cada vez se vaya a ejecutar una instrucción.
	 * Por ejemplo: ejecucion_instruccion("ABCD", "soy", 1, "parametro");
	 *
	 * @param  mnemonico  Nombre de la instrucción a ejecutar.
	 * @param  parametros  Parametros de la instrucción a ejecutar.
	 */
	void ejecucion_instruccion(char* mnemonico, t_list* parametros);

	/**
	 * Debe invocarse cada vez que ocura algún cambio en alguno de los
	 * registros de la CPU (una vez por instruccion a ejecutar, luego de
	 * llamar a ejecucion_instruccion()).
	 *
	 * @param  registros  Estructura conteniendo cada uno de los registros de la CPU.
	 */
	void cambio_registros(t_registros_cpu registros);

	//-------------------------------------------------
	//Retrocompatibilidad con el ejemplo del enunciado:
	void ejecucion_hilo(t_hilo* hilo, uint32_t quantum);

/* Fin_Logs */

#endif /* CPU_H_ */
