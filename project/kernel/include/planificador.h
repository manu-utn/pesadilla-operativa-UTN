#ifndef __PLANIFICADOR__H
#define __PLANIFICADOR__H
#include <commons/log.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include "dir.h"

#define MODULO "kernel"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_SERVIDOR_CFG DIR_BASE MODULO "/config/kernel.cfg"

typedef enum { NEW, READY, EXEC, BLOCKED, SUSBLOCKED, SUSREADY, FINISHED } t_pcb_estado;

// TODO: definir atributos: instrucciones y tabla de paginas
typedef struct {
  uint32_t *socket;
  uint32_t pid;
  uint32_t tamanio;
  uint32_t estimacion_rafaga;
  uint32_t program_counter;
  t_pcb_estado estado;
} t_pcb;

typedef struct {
  t_list *lista;
  sem_t instanciasDisponibles;
  pthread_mutex_t mutex;
} t_cola_planificacion;

t_log *logger;

t_cola_planificacion *COLA_NEW;
t_cola_planificacion *COLA_READY;
t_cola_planificacion *COLA_BLOCKED;
t_cola_planificacion *COLA_SUSREADY;
t_cola_planificacion *COLA_SUSBLOCKED;

void *iniciar_corto_plazo();
void *iniciar_largo_plazo();
void *iniciar_mediano_plazo();

int pcb_get_posicion(t_pcb *pcb, t_list *lista);

void agregar_pcb_a_cola(t_pcb *pcb, t_cola_planificacion *cola);
void cambiar_estado_pcb(t_pcb *pcb, t_pcb_estado nuevoEstado);
void pasar_de_blocked_a_ready(t_pcb *pcb);
void pasar_de_susblocked_a_susready(t_pcb *pcb);
void remover_pcb_de_cola(t_pcb *pcb, t_cola_planificacion *cola);
#endif
