
#include <commons/log.h>
#include <commons/config.h>
#include "execution_unit.h"

void inicializar_CPU(char *path, uint32_t *retardo);

int main(int argc, char **argv) {

	uint32_t retardo;
	char *operation_code;

	inicializar_CPU(argv[1],&retardo);

	while(true) {

		obtener_siguiente_hilo();				/* Solicita un nuevo hilo para ejecutar (TCB y quantum) al Kernel */

		ejecucion_hilo(&Hilo,Quantum);			/* LOG */

		eu_cargar_registros();

		while(Quantum || KernelMode) {

			if(Execution_State != CPU_ABORT) {
				eu_fetch_instruccion(operation_code);
				eu_decode(operation_code);
				eu_ejecutar(operation_code,retardo);
				avanzar_puntero_instruccion();
				eu_actualizar_registros();
			}
		}

		devolver_hilo();	/* Devuelve el hilo al kernel */

		fin_ejecucion();	/* LOG */
	}

	return EXIT_SUCCESS;

}

void inicializar_CPU(char *path, uint32_t *retardo) {
	t_config* config = config_create(path);

	*retardo = config_get_int_value(config,"RETARDO");
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

	MapRegistros['A'-'A'] = &Registros.registros_programacion[0];
	MapRegistros['B'-'A'] = &Registros.registros_programacion[1];
	MapRegistros['C'-'A'] = &Registros.registros_programacion[2];
	MapRegistros['D'-'A'] = &Registros.registros_programacion[3];
	MapRegistros['E'-'A'] = &Registros.registros_programacion[4];

	MapRegistros['K'-'A'] = &Registros.K;
	MapRegistros['I'-'A'] = &Registros.I;
	MapRegistros['M'-'A'] = &Registros.M;
	MapRegistros['P'-'A'] = &Registros.P;
	MapRegistros['S'-'A'] = &Registros.S;
	MapRegistros['X'-'A'] = &Registros.X;

	inicializar_tabla_instrucciones();

}
