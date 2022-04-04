#ifndef UTILS_CLIENTE_H_
#define UTILS_CLIENTE_H_

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

t_log *logger;

typedef struct {
  int size;
  void *stream;
} t_buffer;

typedef struct {
  op_code codigo_operacion;
  t_buffer *buffer;
} t_paquete;

int conectar_a_servidor(char *ip, char *puerto);
void enviar_mensaje(char *mensaje, int socket_cliente);
t_paquete *crear_paquete(void);
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete *paquete);

#endif
