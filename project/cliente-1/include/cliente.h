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

t_log *iniciar_logger(void);
void leer_consola(t_log *);
void paquete(int);
void terminar_programa(int, t_log *, t_config *);

#endif
