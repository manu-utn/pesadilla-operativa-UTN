#ifndef __MY_STATIC_LIB_H__
#define __MY_STATIC_LIB_H__

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef enum { MENSAJE, PAQUETE } op_code;
t_config* iniciar_config(char*);
#endif
