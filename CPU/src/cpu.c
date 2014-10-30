
#include <commons/log.h>
#include <commons/config.h>
#include "execution_unit.h"

int main(int argc, char **argv) {

	char *operation_code;
	t_config* config = config_create(argv[1]);

	uint32_t retardo = config_get_int_value(config,"RETARDO");
	uint16_t puertoMSP = config_get_int_value(config,"PUERTO_MSP");
	char *direccionIpMSP = config_get_string_value(config,"IP_MSP");
	uint16_t puertoKernel = config_get_int_value(config,"PUERTO_KERNEL");
	char *direccionIpKernel = config_get_string_value(config,"IP_KERNEL");

	if((Kernel = client_socket(direccionIpKernel, puertoKernel)) < 0) {
		puts("ERROR: No se pudo conectar al Kernel.");
		exit(EXIT_FAILURE);
	}

	t_msg *handshake = id_message(CPU_CONNECT);
	enviar_mensaje(Kernel, handshake);
	destroy_message(handshake);

	if((MSP = client_socket(direccionIpMSP, puertoMSP)) < 0) {
		puts("ERROR: No se pudo conectar a la MSP.");
		exit(EXIT_FAILURE);
	}

	config_destroy(config);

	inicializar_tabla_instrucciones();

	while(true) {

		obtener_siguiente_hilo();				/* Solicita un nuevo hilo para ejecutar (TCB y quantum) al Kernel */

		ejecucion_hilo(&Hilo,Quantum);

		eu_cargar_registros();

		while(Quantum || Registros.K){

			eu_fetch_instruccion(operation_code);
			eu_decode(operation_code);
			eu_ejecutar(operation_code,retardo);
			avanzar_puntero_instruccion();
			eu_actualizar_registros();

		}

		devolver_hilo();	/* Devuelve el hilo al kernel */

		fin_ejecucion();
	}

	return EXIT_SUCCESS;

}

void inicializar_tabla_instrucciones(void){

	SetInstruccionesDeUsuario = dictionary_create();
	SetInstruccionesProtegidas = dictionary_create();

	dictionary_put(SetInstruccionesDeUsuario,"LOAD",load);
	dictionary_put(SetInstruccionesDeUsuario,"GETM",getm);
	dictionary_put(SetInstruccionesDeUsuario,"SETM",setm);
	dictionary_put(SetInstruccionesDeUsuario,"MOVR",movr);
	dictionary_put(SetInstruccionesDeUsuario,"ADDR",addr);
	dictionary_put(SetInstruccionesDeUsuario,"SUBR",subr);
	dictionary_put(SetInstruccionesDeUsuario,"MULR",mulr);
	dictionary_put(SetInstruccionesDeUsuario,"MODR",modr);
	dictionary_put(SetInstruccionesDeUsuario,"DIVR",divr);
	dictionary_put(SetInstruccionesDeUsuario,"INCR",incr);
	dictionary_put(SetInstruccionesDeUsuario,"DECR",decr);
	dictionary_put(SetInstruccionesDeUsuario,"COMP",comp);
	dictionary_put(SetInstruccionesDeUsuario,"CGEQ",cgeq);
	dictionary_put(SetInstruccionesDeUsuario,"CLEQ",cleq);
	dictionary_put(SetInstruccionesDeUsuario,"GOTO",eso_goto);
	dictionary_put(SetInstruccionesDeUsuario,"JMPZ",jmpz);
	dictionary_put(SetInstruccionesDeUsuario,"JPNZ",jpnz);
	dictionary_put(SetInstruccionesDeUsuario,"INTE",inte);
	dictionary_put(SetInstruccionesDeUsuario,"SHIF",shif);
	dictionary_put(SetInstruccionesDeUsuario,"NOPP",nopp);
	dictionary_put(SetInstruccionesDeUsuario,"PUSH",eso_push);
	dictionary_put(SetInstruccionesDeUsuario,"TAKE",take);
	dictionary_put(SetInstruccionesDeUsuario,"XXXX",xxxx);
/*
	dictionary_put(SetInstruccionesProtegidas,"MALC");
	dictionary_put(SetInstruccionesProtegidas,"FREE");
*/
	dictionary_put(SetInstruccionesProtegidas,"INNN",innn);
	dictionary_put(SetInstruccionesProtegidas,"INNC",innc);
	dictionary_put(SetInstruccionesProtegidas,"OUTN",outn);
	dictionary_put(SetInstruccionesProtegidas,"OUTC",outc);
	dictionary_put(SetInstruccionesProtegidas,"CREA",crea);
	dictionary_put(SetInstruccionesProtegidas,"JOIN",join);
	dictionary_put(SetInstruccionesProtegidas,"BLOCK",block);

}
