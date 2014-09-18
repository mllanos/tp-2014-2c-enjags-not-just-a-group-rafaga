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
#include "../../libs/utiles/utiles/utiles.h"
#include "execution_unit.h"
#define PATH_ARCHIVO_CONF "../config/cpu.conf"

/*dummy*/
void imprimir_tcb(void);
int conectar_a_kernel(void);
int conectar_a_msp(void);
/*dummys*/

#endif /* CPU_H_ */
