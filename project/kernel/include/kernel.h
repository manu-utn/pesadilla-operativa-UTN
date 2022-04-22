#ifndef __KERNEL__H
#define __KERNEL__H
#include <libstatic.h>
#include <semaphore.h>

t_config* config;

int es_esta_instruccion(char* identificador, char** params);
void enviar_instruccion(int socket, op_code codigo_operacion, char** params);
int conectarse_a_cpu(char* conexion_puerto);
void* escuchar_conexiones_entrantes();
#endif
