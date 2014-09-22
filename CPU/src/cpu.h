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
#include "execution_unit.h"

/*dummy*/
void imprimir_tcb(void);
int conectar_a_kernel(char *direccionIP,uint16_t puerto);
int conectar_a_msp(char *direccionIP,uint16_t puerto);
/*dummys*/

#endif /* CPU_H_ */
