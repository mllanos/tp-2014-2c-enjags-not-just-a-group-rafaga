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
	if(kernel_fd == -2) {
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
	while(status) {
		t_msg *recibido = recibir_mensaje(kernel_fd);
		status = interpret_message(recibido);
		destroy_message(recibido);
	}
}

int interpret_message(t_msg *recibido)
{
	char str_input[255];
	char *str_tmp;
	long num_input;
	t_msg *msg;

	log_trace(logger, "RECIBIDO: %s", recibido->stream);

	switch(recibido->header.id) {
		case NUMERIC_INPUT:
			puts("Ingrese un valor numerico.");
			scanf("%ld", &num_input);
			clean_stdin_buffer();
			str_tmp = string_itoa(num_input);
			msg = string_message(STRING_OUTPUT, str_tmp, 0);
			enviar_mensaje(kernel_fd, msg);
			destroy_message(msg);
			free(str_tmp);
			break;
		case STRING_INPUT:
			puts("Ingrese un literal cadena.");
			msg = string_message(STRING_OUTPUT, fgets(str_input, sizeof(str_input), stdin), 0);
			enviar_mensaje(kernel_fd, msg);
			destroy_message(msg);
			break;
		case STRING_OUTPUT:
			puts("Mensaje recibido:");
			puts(recibido->stream);
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
