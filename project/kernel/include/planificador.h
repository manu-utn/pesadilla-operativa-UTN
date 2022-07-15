#ifndef __PLANIFICADOR__H
#define __PLANIFICADOR__H

#include "kernel.h"

#define MODULO "kernel"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_SERVIDOR_CFG DIR_BASE MODULO "/config/kernel.cfg"

int SOCKET_CONEXION_DISPATCH;
int SOCKET_CONEXION_MEMORIA;

// requieren usar `extern` porque se utilizan en varios submodulos de kernel
extern t_pcb *PROCESO_EJECUTANDO;
extern int SE_ENVIO_INTERRUPCION;
extern int SE_INDICO_A_PCP_QUE_REPLANIFIQUE;

sem_t HAY_PCB_DESALOJADO;     // semáforo binario
sem_t EJECUTAR_ALGORITMO_PCP; // semaforo binario
sem_t MUTEX_BLOQUEO_SUSPENSION;
sem_t SUSPENSION_EXITOSA;
sem_t INICIALIZACION_ESTRUCTURAS_EXITOSA;
sem_t LIBERACION_RECURSOS_EXITOSA;
sem_t HAY_PCB_FINISH;

int ULTIMO_PID;
t_queue* PCBS_PROCESOS_ENTRANTES;
sem_t HAY_PROCESOS_ENTRANTES;
sem_t NO_HAY_PROCESOS_EN_SUSREADY;

int REFERENCIA_TABLA_RECIBIDA;

struct timespec BEGIN;
struct timespec END;

typedef struct {
  t_list *lista_pcbs;
  sem_t cantidad_procesos;
  pthread_mutex_t mutex;
} t_cola_planificacion;

typedef enum {
  FIFO,
  SRT
} algoritmo_planif;

t_log *logger;

// TODO: Revisar por mejor nombre
sem_t PROCESOS_DISPONIBLES_EN_MEMORIA; // El maximo es el grado de multiprogramacion

t_cola_planificacion *COLA_NEW;
t_cola_planificacion *COLA_READY;
t_cola_planificacion *COLA_BLOCKED;
t_cola_planificacion *COLA_SUSREADY;
t_cola_planificacion *COLA_FINISHED;

void iniciar_planificacion();
void *iniciar_corto_plazo();
void *iniciar_largo_plazo();
void *iniciar_mediano_plazo();

void avisar_a_pcp_que_decida();
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
int obtener_cantidad_procesos_disponibles_en_memoria();
void controlar_procesos_disponibles_en_memoria(int llamado_por_plp);
void liberar_espacio_en_memoria_para_proceso();
void imprimir_cantidad_procesos_disponibles_en_memoria();

t_pcb *elegir_pcb_fifo(t_cola_planificacion *cola);
t_pcb *elegir_pcb_srt(t_cola_planificacion *cola);

t_pcb *pcb_menor_tiempo_restante_de_ejecucion_entre(t_pcb *pcb1, t_pcb *pcb2);
int pcb_tiempo_restante_de_ejecucion(t_pcb *pcb);

t_pcb *elegir_pcb_segun_algoritmo();
bool algoritmo_cargado_es(char *algoritmo);
void liberar_cpu();

void enviar_interrupcion();
bool hay_algun_proceso_ejecutando();
void transicion_ready_a_running(t_pcb *pcb);
void ejecutar_proceso(t_pcb*pcb);
void imprimir_proceso_en_running();
int calcular_estimacion_rafaga(t_pcb *pcb);

void iniciar_conexion_cpu_dispatch();
void *escuchar_conexion_cpu_dispatch();
void *gestor_de_procesos_bloqueados();
void timer_suspension_proceso(t_pcb *pcb);
int conectarse_a_memoria();
void escuchar_conexion_con_memoria();

void evaluar_replanificacion_pcp();

// CONFIGS
int obtener_tiempo_maximo_bloqueado();
char *obtener_algoritmo_cargado();
double obtener_alfa_por_config();
int obtener_grado_multiprogramacion_por_config();
int obtener_estimacion_inicial_por_config();
char *obtener_ip_de_modulo_por_config(char *nombreDelModulo);
char *obtener_puerto_de_modulo_por_config(char *nombreDelModulo);
#endif
