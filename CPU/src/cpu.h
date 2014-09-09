/*
 * cpu.h
 *
 *  Created on: 05/09/2014
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

struct {
	int tcb;			//tipo tcb no int, cuando tenga el struct tcb lo cambio
	uint8_t quantum;
} typedef hilo_t;

#endif /* CPU_H_ */
