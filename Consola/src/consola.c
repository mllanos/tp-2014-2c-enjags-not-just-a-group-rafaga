#include "consola.h"

int main(int argc, char **argv)
{

	initialize();
	kernel_connect(argv[1]);
	receive_messages();
	finalize();
	return EXIT_SUCCESS;
}

void initialize(void)
{
	logger = log_create(LOG_PATH, "Consola", false, LOG_LEVEL_TRACE);
	config = config_create(CONF_PATH);
}

void kernel_connect(char *beso_path)
{
	log_trace(logger, "Bienvenido al proceso Consola.");

	kernel_fd = client_socket(get_ip_kernel(), get_puerto_kernel());
	if (kernel_fd < 0) {
		log_trace(logger, "Error al conectar con Kernel.");
		exit(EXIT_FAILURE);
	}

	t_msg *beso_msg = beso_message(INIT_CONSOLE, beso_path, 0);
	enviar_mensaje(kernel_fd, beso_msg);
	destroy_message(beso_msg);
}

void receive_messages(void)
{
	int status = 1;
	t_msg *recibido;
	
	while (status) {
		recibido = recibir_mensaje(kernel_fd);
		if(recibido == NULL) {
			log_trace(logger, "El Kernel ha finalizado su ejecucion.");
			break;
		}
		status = interpret_message(recibido);
		destroy_message(recibido);
	}
}

int interpret_message(t_msg *recibido)
{
	char *aux = NULL;
	char *str_input = NULL;
	int32_t num_input;
	t_msg *msg = NULL;

	log_trace(logger, "RECIBIDO: %s", recibido->stream);

	switch (recibido->header.id) {
		case NUMERIC_INPUT:
			puts("Ingrese un valor numerico.");

			while(scanf(A_LINE, &str_input) == 0 || sscanf(str_input, "%d%1s", &num_input, aux) != 1) {
				getchar();
				puts("Argumentos inválidos, intentelo de nuevo.");
			}
			getchar();

			/* Adjuntamos el cpu_sock_fd del mensaje recibido. */
			msg = argv_message(REPLY_NUMERIC_INPUT, 2, recibido->argv[0], num_input);
			enviar_mensaje(kernel_fd, msg);
			destroy_message(msg);
			break;
		case STRING_INPUT:
			printf("Ingrese un literal cadena de tamaño: %u.\n",recibido->argv[1]);
			/* Adjuntamos el cpu_sock_fd del mensaje recibido. */

			while(scanf(A_LINE, &str_input) == 0) {
				getchar();
				puts("Argumentos inválidos, intentelo de nuevo.");
			}
			getchar();

			if(strlen(str_input) > recibido->argv[1])
				str_input[recibido->argv[1]] = '\0';

			msg = string_message(REPLY_STRING_INPUT, str_input, 1, recibido->argv[0]);
			enviar_mensaje(kernel_fd, msg);
			destroy_message(msg);
			free(str_input);
			break;
		case NUMERIC_OUTPUT:
			printf("Número recibido: %d\n", recibido->argv[0]);
			break;
		case STRING_OUTPUT:
			printf("Mensaje recibido: %s\n", recibido->stream);
			break;
		case KILL_CONSOLE:
			puts(recibido->stream);
			return 0;
		default:
			errno = EBADMSG;
			perror("interpret_message");
			exit(EXIT_FAILURE);
	}

	return 1;
}

void finalize(void)
{
	log_destroy(logger);
	config_destroy(config);
}
