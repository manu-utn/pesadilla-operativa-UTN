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
t_config *config;

pcb_timer_t TIMER;

#define MODULO "servidor-1"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_SERVIDOR_CFG DIR_BASE MODULO "/config/servidor.cfg"

void* iniciar_conexion_interrupt();
void* iniciar_conexion_dispatch();
void desalojar_y_enviar_proceso_en_ejecucion();
void* escuchar_conexiones_entrantes(void* args);
void* escuchar_nueva_conexion(void* args);
void* escuchar_conexiones_entrantes_en_interrupt();
void iniciar_ciclo_de_instruccion(t_pcb* pcb);
bool es_esta_instruccion(t_instruccion* instruccion, char* identificador);
void iniciar_ciclo_de_instruccion(t_pcb* pcb);
int instruccion_obtener_parametro(t_instruccion* instruccion, int numero_parametro);
void validar_operacion_io(t_pcb* pcb, t_instruccion* instruccion);
#endif
