#ifndef CONSOLA_H_
#define CONSOLA_H_

	#include <string.h>
	#include <stdlib.h>
	#include <stdio.h>

	#include "string.h"

	void mostrarTexto(char* mensaje);
	void mostrarValorString(char* mensaje, char* string);
	void mostrarValorChar(char* mensaje, char caracter);
	void mostrarValorEntero(char* mensaje, int entero);
	void mostrarValorFloat(char* mensaje, float f);
	void mostrarValorDouble(char* mensaje, double d);

	char* concatenarChar(char* string, char caracter);
	char* concatenarIpSocket(char* string, char* ip, int socket);

#endif /* CONSOLA_H_ */
