#include "consola.h"

int main(int argc, char **argv)
{

	initialize();
	kernel_connect(argv[1]);
	receive_messages();
	finalize();
	return 0;
}

void initialize(void)
{
	logger = log_create(LOG_PATH, "Consola", 1, LOG_LEVEL_TRACE);
	config = config_create(CONF_PATH);
}

void kernel_connect(char *beso_path)
{
	log_trace(logger, "Bienvenido al proceso Consola.");

	kernel_fd = client_socket(get_ip_kernel(), get_puerto_kernel());
	if (kernel_fd == -2) {
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
		if((recibido = recibir_mensaje(kernel_fd)) == NULL) {
			puts("ERROR: Se ha perdido la conexión con el Kernel.");
			exit(EXIT_FAILURE);
		}
		status = interpret_message(recibido);
		destroy_message(recibido);
	}
}

int interpret_message(t_msg *recibido)
{
	char *str_input;
	int32_t num_input;
	t_msg *msg;

	log_trace(logger, "RECIBIDO: %s", recibido->stream);

	switch (recibido->header.id) {
		case NUMERIC_INPUT:
			puts("Ingrese un valor numerico.");
			scanf("%d", &num_input);
			clean_stdin_buffer();
			/* Adjuntamos el cpu_sock_fd del mensaje recibido. */
			msg = argv_message(REPLY_NUMERIC_INPUT, 2, recibido->argv[0],num_input);
			enviar_mensaje(kernel_fd, msg);
			destroy_message(msg);
			break;
		case STRING_INPUT:
			puts("Ingrese un literal cadena.");
			/* Adjuntamos el cpu_sock_fd del mensaje recibido. */
			str_input = malloc(recibido->argv[0]+1);
			msg = string_message(REPLY_STRING_INPUT, fgets(str_input, recibido->argv[0], stdin), 1, recibido->argv[1]);
			enviar_mensaje(kernel_fd, msg);
			destroy_message(msg);
			break;
		case NUMERIC_OUTPUT:
			printf("Número recibido: %d\n", recibido->argv[0]);
		case STRING_OUTPUT:
			printf("Mensaje recibido: %s\n", recibido->stream);
			break;
		case KILL_CONSOLE:
			printf("Cerrando consola. Razon: %s\n", msg->stream);
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
	close(kernel_fd);
}


char *get_ip_kernel(void)
{
	return config_get_string_value(config, "IP_KERNEL");
}

int get_puerto_kernel(void)
{
	return config_get_int_value(config, "PUERTO_KERNEL");
}
