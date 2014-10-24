#include <stdio.h>
#include <stdlib.h>

int esperarComando(void);

int main(void) {

	unsigned int num,desp,resultado;	

	while(1) {

		switch(esperarComando()) {
		case 1:
			scanf("%u%u",&num,&desp);
			resultado = num >> desp;
			printf("Resultado: %u\n",resultado);
			break;
		case -1:
			scanf("%u%u",&num,&desp);
			resultado = num << desp;
			printf("Resultado: %u\n",resultado);
			break;
		default:
			puts("COMANDO INVÁLIDO");
		}

	}

	return 0;

}

int esperarComando(void) {

	char *command;
	int idCommand;

	scanf("%ms",&command);

	if(!strcmp(command,"right"))
		idCommand = 1;
	else if(!strcmp(command,"left"))
		idCommand = -1;
	else
		idCommand = 0;

	free(command);

	return idCommand;

}

