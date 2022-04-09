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

typedef enum { MENSAJE=0, PAQUETE=1} op_code;

typedef struct {
  int size;
  void* stream;
} t_buffer;

typedef struct {
  op_code codigo_operacion;
  t_buffer* buffer;
} t_paquete;

t_config* iniciar_config(char*);
t_log* iniciar_logger(char* archivo, char* nombre);

t_buffer* crear_mensaje(char* texto);
t_paquete* paquete_create();
t_buffer* empty_buffer();
int get_paquete_size(t_paquete* paquete);

void* paquete_add_mensaje(t_paquete* paquete, t_buffer* nuevo_mensaje);

void mensaje_destroy(t_buffer* mensaje);
void paquete_destroy(t_paquete* paquete);

void liberar_conexion(int socket);
void terminar_programa(int conexion, t_log* logger, t_config* config);
#endif
