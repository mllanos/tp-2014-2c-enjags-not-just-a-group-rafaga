#ifndef CPU_H_
#define CPU_H_

	#include "panel.h"

	typedef struct {
		int32_t registros_programacion[5]; //A, B, C, D y E
		uint32_t M; //Base de segmento de código
		uint32_t P; //Puntero de instrucción
		uint32_t X; //Base del segmento de Stack
		uint32_t S; //Cursor de stack
		uint32_t K; //Kernel Mode
		uint32_t I; //PID
	} t_registros_cpu;

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
	 * @param  parametros  Parametros de la instrucción. a ejecutar.
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

#endif
