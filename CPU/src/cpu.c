/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */

#include "cpu.h"

int main(int argc, char **argv) {

	//Levantar archivo de configuracion
	t_config* config = config_create(argv[1]);
	uint16_t puerto_kernel = config_get_int_value(config,"PUERTO_KERNEL");
	char *direccionIP_kernel = config_get_string_value(config,"IP_KERNEL");
	uint16_t puerto_msp = config_get_int_value(config,"PUERTO_MSP");
	char *direccionIP_msp = config_get_string_value(config,"IP_MSP");
	int retardo = config_get_int_value(config,"RETARDO");
	//FIN levantar archivo de configuracion

	if(conectar_a_kernel(direccionIP_kernel,puerto_kernel) == -1){
		//log("error_conectar_kernel");
		puts("Hubo un error al conectar al Kernel. Abortado.");	//imprimir el mensaje en la consola donde se ejecuta el proceso, no la consola remota
		exit(EXIT_FAILURE);
	}

	if(conectar_a_msp(direccionIP_msp,puerto_msp) == -1){	//cambiar utiles para poder validar el error yo
		//log("error_conectar_msp");
		puts("Hubo un error al conectar a la MSP. Abortado.");
		exit(EXIT_FAILURE);
	}

	inicializar_tabla_instrucciones();

	/*dummy*/
	tcb = fopen("BESO/A.bc","r+");
	/*dummy*/

	while(1){

		obtener_siguiente_hilo();								//solicita un nuevo hilo para ejecutar (TCB y quantum) al Kernel.
		eu_cargar_registros();

		while(quantum || registros.K){

			eu_fetch_instruccion();
			eu_decode();
			eu_ejecutar(retardo);
			avanzar_puntero_instruccion(instruccion_size);
			eu_actualizar_registros();
			/*dummy*/
			imprimir_tcb();
			/*dummy*/
		}

		//devolver_hilo();										//devuelve el hilo al kernel.

	}

	return EXIT_SUCCESS;

}

int conectar_a_kernel(char *direccionIP,uint16_t puerto){

	kernel = client_socket(direccionIP,puerto);
	return 0;
}

int conectar_a_msp(char *direccionIP,uint16_t puerto){

	msp = client_socket(direccionIP,puerto);
	return 0;
}

/*dummy*/
void imprimir_tcb(void) {

	int i;
	for(i = 0;i < 5; ++i){
		printf("Registro %c. Valor: %4d\n",('A'+i), hilo.registros[i]);
		fflush(stdout);
	}
	puts("\n");
/*
	printf("Registro M Valor: %4d\n", hilo.segmento_codigo);
	printf("Registro P Valor: %4d\n", hilo.puntero_instruccion);
	printf("Registro X Valor: %4d\n", hilo.base_stack);
	printf("Registro S Valor: %4d\n", hilo.cursor_stack);
	printf("Registro K Valor: %4d\n", hilo.kernel_mode);
	printf("Registro I Valor: %4d\n\n", hilo.pid);
*/
}
/*dummys*/
