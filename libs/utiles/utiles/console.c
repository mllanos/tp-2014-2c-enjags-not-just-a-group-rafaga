#include "console.h"

void mostrarTexto(char* mensaje){
	printf("%s \n", mensaje);
}

void mostrarValorString(char* mensaje, char* string){
	printf("%s: %s \n", mensaje, string);
}

void mostrarValorChar(char* mensaje, char caracter){
	printf("%s: %c \n", mensaje, caracter);
}

void mostrarValorEntero(char* mensaje, int entero){
	printf("%s: %d \n", mensaje, entero);
}

void mostrarValorFloat(char* mensaje, float f){
	printf("%s: %f \n", mensaje, f);
}

void mostrarValorDouble(char* mensaje, double d){
	printf("%s: %lf \n", mensaje, d);
}
