/*
 * consola.h
 *
 *  Created on: 05/09/2014
 *      Author: utnso
 */

#ifndef CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <utiles/utiles.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define CONF_PATH "config/consola.conf"
#define LOG_PATH "log/consola.log"

 int kernel_fd;
 t_config *config;
 t_log *logger;

 void initialize(void);
 void kernel_connect(char *beso_path);
 void receive_messages(void);
 int interpret_message(t_msg *recibido);
 void finalize(void);

 char *get_ip_kernel(void);
 int get_puerto_kernel(void);

#define CONSOLA_H_

#endif /* CONSOLA_H_ */
