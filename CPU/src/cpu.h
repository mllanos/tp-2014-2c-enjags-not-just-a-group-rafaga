/*
 * cpu.h
 *
 *  Created on: 05/09/2014
 *      Author: utnso
 */

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

typedef struct {
	char mnemonico[5];
	//t_list parametros;
	uint32_t parametros[3];
} t_instruccion;

struct {
	char mnemonico[5];
	size_t cantidad_parametros;
	bool protegida;
	void (*rutina) (void);
} set_instrucciones[34];

void avanzar_puntero_instruccion(size_t desplazamiento);
void actualizar_registros_tcb(void);
void cargar_registros(void);
void ejecutar_instruccion(void);

/*Loggeo de eventos*/
/*
void comienzo_ejecucion(t_hilo* hilo, uint32_t quantum);
void fin_ejecucion();
void ejecucion_instruccion(char* mnemonico, t_list* parametros);
void cambio_registros(t_registros_cpu registros);
void ejecucion_hilo(t_hilo* hilo, uint32_t quantum);
*/
#endif /* CPU_H_ */
