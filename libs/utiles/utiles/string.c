#include <stdlib.h>
#include <string.h>
#include <commons/string.h>

#include "string.h"

int longitud_string(int numero){
	int i = 1;
	while ((numero / 10) > 0){
		numero = numero / 10;
		i++;
	}
	return i;
}

int longitud_array(char** array){
	int longitud = 0;
	while(array[longitud]!= '\0'){
		longitud++;

	}
	return longitud;
}

char* obtenerDatoString(char* str, int cantInicial, int cantFinal){
	int longitud = strlen(str) - cantFinal;
	char* dato = string_substring_until(str, longitud);
	dato = string_substring_from(dato, cantInicial);
	return dato;
}

char* convertir_array_a_string(char **array){
	int elem = 0;
	int i = 0;
	char* cadena;
	while(array[elem]!=NULL){
		elem++;
	}
	cadena =(char*)malloc(elem*sizeof(char));
	for(i = 0; i < elem; i++){
		cadena[i] = *array[i];
	}
	cadena[elem] = '\0';
	return cadena;
}

void liberar_array_string(char **array) {
	int i = 0;
	while (array[i] != NULL ) {
		free(array[i]);
		i++;
	}
	free(array);
}

char* eliminarCorchetes(char* array){
	char* nuevoString = string_new();
	char** arraySinCorchetes = string_get_string_as_array(array);
	int i = 0;
	while (arraySinCorchetes[i] != NULL){
		string_append(&nuevoString, arraySinCorchetes[i]);
		string_append(&nuevoString, ",");
		i++;
	}
	int longitud = strlen(nuevoString) - 1;
	nuevoString = string_substring_until(nuevoString, longitud);
 	return nuevoString;
}
