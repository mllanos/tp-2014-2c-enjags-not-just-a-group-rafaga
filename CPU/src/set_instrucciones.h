/*
 * instrucciones-eso.h
 *
 *  Created on: 09/09/2014
 *      Author: matias
 */

#ifndef INSTRUCCIONES_ESO_H_
#define INSTRUCCIONES_ESO_H_

/*Definición de los tipos t_instruccion y t_operandos*/
enum {REGISTRO,NUMERO,DIRECCION} typedef t_operandos;

struct {
	char mnemonico[5];
	void (*rutina) (void);
} typedef t_instruccion;
/*FIN_Definición de los tipos t_instruccion y t_operandos*/

/*Variables Globales*/
t_instruccion tabla_instrucciones[34];
/*FIN_Variables Globales*/

/*Funciones del Set de Instrucciones*/
void inicializar_tabla_instrucciones(void);
/*FIN_Funciones del Set de Instrucciones*/

#endif /* INSTRUCCIONES_ESO_H_ */
