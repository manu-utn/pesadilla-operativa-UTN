#ifndef UTILS_SERVIDOR_H_
#define UTILS_SERVIDOR_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "libstatic.h"
#include "serializado.h"

t_log *logger;

void *recibir_buffer(int *, int);
int iniciar_servidor(char *, char *);
int esperar_cliente(int);
int recibir(int socket_cliente, t_buffer *mensaje);
t_paquete *recibir_paquete(int socket_cliente);
t_buffer* recibir_mensaje(int);
int recibir_operacion(int);

#endif
