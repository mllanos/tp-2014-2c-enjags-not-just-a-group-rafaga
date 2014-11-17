/*
 * consola.h
 *
 *  Created on: 05/09/2014
 *      Author: utnso
 */

#ifndef CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <commons/log.h>
#include <utiles/utiles.h>
#include <commons/string.h>
#include <commons/config.h>

#define CONF_PATH "config/consola.conf"
#define LOG_PATH "log/consola.log"
#define A_LINE "%m[^\n]"

#define get_ip_kernel() config_get_string_value(config, "IP_KERNEL")
#define get_puerto_kernel() config_get_int_value(config, "PUERTO_KERNEL")

int kernel_fd;
t_config *config;
t_log *logger;

void initialize(void);
void kernel_connect(char *beso_path);
void receive_messages(void);
int interpret_message(t_msg *recibido);
void finalize(void);
 
#define CONSOLA_H_

#endif /* CONSOLA_H_ */
