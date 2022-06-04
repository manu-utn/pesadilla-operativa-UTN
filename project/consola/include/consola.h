#ifndef __CONSOLA__H
#define __CONSOLA__H

#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>

t_log* logger;

#define MODULO "consola"
#define DIR_CONFIG DIR_BASE MODULO "/config"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_CONSOLA_CFG DIR_BASE MODULO "/config/consola.cfg"

int conectarse_a_kernel();
t_list* obtener_instrucciones_de_archivo(char* ruta_archivo);
void escuchar_a_kernel(int socket_servidor);
#endif
