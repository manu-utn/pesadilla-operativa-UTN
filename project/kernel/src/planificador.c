#include "planificador.h"

void iniciar_planificacion() {
  pthread_t th;
  ULTIMO_PID = 0;

  inicializar_cola(COLA_NEW);
  inicializar_cola(COLA_READY);
  inicializar_cola(COLA_BLOCKED);
  inicializar_cola(COLA_SUSREADY);
  inicializar_cola(COLA_SUSBLOCKED);

  pthread_create(&th, NULL, iniciar_largo_plazo, NULL);
  pthread_detach(th);
}

// TODO: definir
void *iniciar_corto_plazo() {
  return NULL;
}

// TODO: definir
void *iniciar_largo_plazo() {
  log_info(logger, "Planificador de Largo Plazo: Ejecutando...");

  while (1) {
    sem_wait(&PROCESOS_PENDIENTES_A_INGRESAR);
  }

  return NULL;
}

// TODO: definir
void *iniciar_mediano_plazo() {
  return NULL;
}

int pcb_get_posicion(t_pcb *pcb, t_list *lista) {
  for (int posicion = 0; posicion < list_size(lista); posicion++) {
    if (pcb == (t_pcb *)list_get(lista, posicion)) {
      return posicion;
    }
  }
  return -1;
}

void agregar_pcb_a_cola(t_pcb *pcb, t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));
  list_add(cola->lista_pcbs, pcb);
  pthread_mutex_unlock(&(cola->mutex));
}

void remover_pcb_de_cola(t_pcb *pcb, t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));
  int posicion = pcb_get_posicion(pcb, cola->lista_pcbs);

  if (posicion != -1) {
    list_remove(cola->lista_pcbs, posicion);
  } else {
    log_error(logger, "No existe tal elemento en la cola");
  }

  pthread_mutex_unlock(&(cola->mutex));
}

void cambiar_estado_pcb(t_pcb *pcb, t_pcb_estado nuevoEstado) {
  pcb->estado = nuevoEstado;
}

// TODO: Añadir un signal
void transicion_blocked_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_BLOCKED);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);
}

// TODO: Añadir un signal
void transicion_susblocked_a_susready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_SUSBLOCKED);
  cambiar_estado_pcb(pcb, SUSREADY);
  agregar_pcb_a_cola(pcb, COLA_SUSREADY);
}

void inicializar_cola(t_cola_planificacion *cola) {
  int sem_init_valor = 0;

  cola = (t_cola_planificacion *)malloc(sizeof(t_cola_planificacion));
  cola->lista_pcbs = list_create();

  pthread_mutex_init(&(cola->mutex), NULL);
  sem_init(&(cola->instancias_disponibles), 0, sem_init_valor);
}


// TODO: unificar ambos casos para evitar repeticion del codigo
t_pcb *select_pcb_by_fifo(t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));
  t_pcb *primer_pcb = (t_pcb *)list_get(cola->lista_pcbs, 0);
  pthread_mutex_unlock(&(cola->mutex));
  remover_pcb_de_cola(primer_pcb, cola);
  return primer_pcb;
}

t_pcb *select_pcb_by_srt(t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));
  t_pcb *srt_pcb = (t_pcb *)list_get_minimum(cola->lista_pcbs, 0);
  pthread_mutex_unlock(&(cola->mutex));
  remover_pcb_de_cola(srt_pcb, cola);
  return srt_pcb;
}

t_pcb *minimum_estimacion_rafaga(t_pcb *pcb1, t_pcb *pcb2) {
  return pcb1->estimacion_rafaga >= pcb2->estimacion_rafaga ? pcb1 : pcb2;
}