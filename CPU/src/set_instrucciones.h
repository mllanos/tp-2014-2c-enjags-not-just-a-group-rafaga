/*
 * instrucciones-eso.h
 *
 *  Created on: 09/09/2014
 *      Author: matias
 */

#ifndef INSTRUCCIONES_ESO_H_
#define INSTRUCCIONES_ESO_H_

struct {
	char mnemonico[5];
	void (*rutina) (void);
} typedef t_instruccion;

enum {REGISTRO,NUMERO,DIRECCION} typedef t_operandos;

void inicializar_tabla_instrucciones(void);

#endif /* INSTRUCCIONES_ESO_H_ */
