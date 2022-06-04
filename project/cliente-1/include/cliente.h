#ifndef __CLIENTE__H
#define __CLIENTE__H

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils-cliente.h"

t_log *logger;

#define MODULO "cliente-1"
#define DIR_CONFIG DIR_BASE MODULO "/config"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_CLIENTE_CFG DIR_BASE MODULO "/config/cliente.cfg"

int conectarse_a_kernel();
t_list* obtener_instrucciones_de_archivo(char* ruta_archivo);
#endif
