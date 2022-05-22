#ifndef __PLANIFICADOR__H
#define __PLANIFICADOR__H
#include <commons/log.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <libstatic.h>
#include "dir.h"
#include "kernel.h"

#define MODULO "kernel"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_SERVIDOR_CFG DIR_BASE MODULO "/config/kernel.cfg"

int ULTIMO_PID;
t_queue* PCBS_PROCESOS_ENTRANTES;
sem_t HAY_PROCESOS_ENTRANTES;
pthread_mutex_t NO_HAY_PROCESOS_EN_SUSREADY;

typedef struct {
  t_list *lista_pcbs;
  sem_t cantidad_procesos;
  // TODO: evaluar si corresponde remover, se cambió por cantidad_procesos
  // sem_t instancias_disponibles;
  pthread_mutex_t mutex;
} t_cola_planificacion;

typedef enum {
  FIFO,
  SRT
} algoritmo_planif;

t_log *logger;

sem_t GRADO_MULTIPROGRAMACION;

t_cola_planificacion *COLA_NEW;
t_cola_planificacion *COLA_READY;
t_cola_planificacion *COLA_BLOCKED;
t_cola_planificacion *COLA_SUSREADY;
t_cola_planificacion *COLA_SUSBLOCKED; // No va a ser vecesaria
t_cola_planificacion *COLA_FINISHED;

void iniciar_planificacion();
void *iniciar_corto_plazo();
void *iniciar_largo_plazo();
void *iniciar_mediano_plazo();

void *plp_pcb_finished();

int pcb_get_posicion(t_pcb *pcb, t_list *lista);

void agregar_pcb_a_cola(t_pcb *pcb, t_cola_planificacion *cola);
void remover_pcb_de_cola(t_pcb *pcb, t_cola_planificacion *cola);
void cambiar_estado_pcb(t_pcb *pcb, t_pcb_estado nuevoEstado);

void enviar_pcb_de_cola_ready_a_cpu();

void transicion_a_new(t_pcb* pcb);
void transicion_new_a_ready(t_pcb *pcb);
void transicion_blocked_a_ready(t_pcb *pcb);
void transicion_blocked_a_susready(t_pcb *pcb);
void transicion_susblocked_a_susready(t_pcb *pcb);
void transicion_susready_a_ready(t_pcb *pcb);
void transicion_running_a_blocked(t_pcb *pcb);
void transicion_running_a_finished(t_pcb *pcb);

t_cola_planificacion* cola_planificacion_create();
void cola_destroy(t_cola_planificacion *cola);

void inicializar_grado_multiprogramacion();
int obtener_grado_multiprogramacion_actual();
//int obtener_grado_multiprogramacion_por_config();
void bajar_grado_multiprogramacion();
void subir_grado_multiprogramacion();
void controlar_grado_multiprogramacion();
void actualizar_grado_multiprogramacion();
void imprimir_grado_multiprogramacion_actual();

t_pcb *elegir_pcb_fifo(t_cola_planificacion *cola);
t_pcb *elegir_pcb_sjf(t_cola_planificacion *cola);

t_pcb *pcb_menor_tiempo_restante_de_ejecucion_entre(t_pcb *pcb1, t_pcb *pcb2);
int pcb_tiempo_restante_de_ejecucion(t_pcb *pcb);

t_pcb *elegir_pcb_segun_algoritmo();
bool algoritmo_cargado_es(char *algoritmo);
void liberar_cpu();

void enviar_interrupcion();
bool hay_algun_proceso_ejecutando();
void transicion_ready_a_running(t_pcb *pcb);
char *obtener_algoritmo_cargado();
void ejecutar_proceso(t_pcb*pcb);
void imprimir_proceso_en_running();
int calcular_estimacion_rafaga(t_pcb *pcb);

void iniciar_conexion_cpu_dispatch();
void *escuchar_conexion_cpu_dispatch();
void *gestor_de_procesos_bloqueados();
void timer_suspension_proceso(t_pcb *pcb);
#endif
