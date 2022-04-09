#ifndef UTILS_SERIALIZADO_H_
#define UTILS_SERIALIZADO_H_

#include <commons/config.h>
#include <commons/log.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "libstatic.h"

t_log *logger;

void *serializar_paquete(t_paquete *paquete);
void **deserializar_paquete(t_paquete *paquete_serializado);
#endif
