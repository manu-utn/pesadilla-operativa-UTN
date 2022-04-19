#ifndef __KERNEL__H
#define __KERNEL__H
#include <libstatic.h>
int es_esta_instruccion(char* identificador, char** params);
void enviar_instruccion(int socket, op_code codigo_operacion, char** params);
int conectarse_a_cpu_dispatch();
#endif
