#ifndef __KERNEL__H
#define __KERNEL__H
#include "dir.h"

// C HEADERS
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>

// COMMONS HEADERS
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>

// OWN HEADERS
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include "xlog.h"
#include <libstatic.h>

t_config* config;

void enviar_instruccion(int socket, op_code codigo_operacion, char** params);
int conectarse_a_cpu(char* conexion_puerto);
void* escuchar_conexiones_entrantes();
void* escuchar_nueva_conexion(void* args);
void asignar_pid(t_pcb* pcb);
#endif
