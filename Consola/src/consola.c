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
	log_trace(logger, "Bienvenido al proceso consola.");

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

		putmsg(recibido);

		status = interpret_message(recibido);

		destroy_message(recibido);
	}
}

int interpret_message(t_msg *recibido)
{
	switch(recibido->header.id) {
		// TODO
		case KILL_CONSOLE:
			log_trace(logger, "Finalizando proceso consola.");
			return 0;
			break;
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

/*
char *direccionIP;
char * puerto;
int sizeArchivo,leido;
int socketDesc;

char* leerUnBESO (char *rutaArchivo, int *tamanioBuffer) {

	FILE *archivoBESO;
	char *buffer;

	archivoBESO=fopen(rutaArchivo,"r");

	if(archivoBESO) {
		fseek(archivoBESO,0,SEEK_END); // me posiciono al final
		sizeArchivo=ftell(archivoBESO);
		*tamanioBuffer = sizeArchivo;
		rewind(archivoBESO); // volver al inicio del archivo
		buffer=malloc(sizeof(char)*sizeArchivo);
		leido=fread(buffer,sizeof(char),sizeArchivo,archivoBESO);
		fclose(archivoBESO);
	} else {
		printf("Error al abrir archivo");
	}
	if (leido != sizeArchivo){ // no se leyo completamente
		free(buffer);
		buffer=NULL;
	}
	return buffer;
}

t_log *logger; //Archivo de log
char path_log[30] = "log/logConsola"; //Path del log

int main (int argc, char **argv)
{
	int tamanioBuffer = 0;
	char* mensaje;
	logger = log_create(path_log, "Consola", 1, LOG_LEVEL_TRACE);
	log_trace(logger, "BIENVENIDO AL PROCESO CONSOLA");
	//Levantar archivo de configuracion
	t_config* config;
	config=config_create(PATH_ARCHIVO_CONF);
	puerto=config_get_string_value(config,"PUERTO_KERNEL");
	direccionIP=config_get_string_value(config,"IP_KERNEL");
	//Fin levantar archivo de configuracion

	mensaje=leerUnBESO("/home/utnso/git/tp-2014-2c-enjags-not-just-a-group/Ensamblador/EjemplosESO/bigStack.txt",&tamanioBuffer);

	socketDesc=socket_cliente(direccionIP,puerto);

	send(socketDesc, mensaje, sizeArchivo + 1, 0);

	if (socketDesc<0) {
		log_trace(logger,"ERROR AL CONECTAR AL KERNEL, FINALIZANDO PROGRAMA");
		exit(-1);
	}

	return 0;
}

*/

