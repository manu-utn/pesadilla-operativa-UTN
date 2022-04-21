#ifndef __PLANIFICADOR__H
#define __PLANIFICADOR__H
#include <commons/log.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <libstatic.h>
#include "dir.h"

#define MODULO "kernel"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_SERVIDOR_CFG DIR_BASE MODULO "/config/kernel.cfg"

int ULTIMO_PID;

typedef struct {
  t_list *lista_pcbs;
  sem_t instancias_disponibles;
  pthread_mutex_t mutex;
} t_cola_planificacion;

t_log *logger;

t_cola_planificacion *COLA_NEW;
t_cola_planificacion *COLA_READY;
t_cola_planificacion *COLA_BLOCKED;
t_cola_planificacion *COLA_SUSREADY;
t_cola_planificacion *COLA_SUSBLOCKED;

sem_t PROCESOS_PENDIENTES_A_INGRESAR;

void iniciar_planificacion();
void *iniciar_corto_plazo();
void *iniciar_largo_plazo();
void *iniciar_mediano_plazo();

int pcb_get_posicion(t_pcb *pcb, t_list *lista);

void agregar_pcb_a_cola(t_pcb *pcb, t_cola_planificacion *cola);
void remover_pcb_de_cola(t_pcb *pcb, t_cola_planificacion *cola);
void cambiar_estado_pcb(t_pcb *pcb, t_pcb_estado nuevoEstado);

void transicion_a_new(t_pcb* pcb);
void transicion_blocked_a_ready(t_pcb *pcb);
void transicion_susblocked_a_susready(t_pcb *pcb);

t_cola_planificacion* cola_planificacion_create();
void cola_destroy(t_cola_planificacion *cola);
#endif
