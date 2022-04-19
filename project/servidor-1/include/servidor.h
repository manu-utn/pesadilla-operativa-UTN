#ifndef __SERVIDOR__H
#define __SERVIDOR__H

#include "utils-servidor.h"
#include "libstatic.h"

#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dir.h"

t_log *logger;

#define MODULO "servidor-1"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_SERVIDOR_CFG DIR_BASE MODULO "/config/servidor.cfg"

#endif
