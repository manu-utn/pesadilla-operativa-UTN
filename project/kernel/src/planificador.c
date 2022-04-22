#include "planificador.h"
#include "kernel.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/string.h>
#include <libstatic.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

void iniciar_planificacion() {
  pthread_t th1, th2;
  ULTIMO_PID = 0;

  inicializar_grado_multiprogramacion();

  COLA_NEW = cola_planificacion_create();
  COLA_READY = cola_planificacion_create();

  // TODO: descomentar a medida que se vayan implementando los planificadores
  /* COLA_BLOCKED = inicializar_cola(COLA_BLOCKED); */
  /* COLA_SUSREADY = inicializar_cola(COLA_SUSREADY); */
  /* COLA_SUSBLOCKED = inicializar_cola(COLA_SUSBLOCKED); */

  pthread_create(&th1, NULL, iniciar_largo_plazo, NULL), pthread_detach(th1);
  pthread_create(&th2, NULL, iniciar_corto_plazo, NULL), pthread_detach(th2);

  // TODO: validar cuando debemos liberar los recursos asignados a las colas de planificación
  // cola_destroy(COLA_NEW);
  // cola_destroy(COLA_READY);
}

// TODO: validar agregando PCBs con instrucciones
void cola_destroy(t_cola_planificacion *cola) {
  list_destroy_and_destroy_elements(cola->lista_pcbs, (void *)pcb_destroy);
  free(cola);
}

// TODO: añadir un logger sólo para este planificador
void *iniciar_corto_plazo() {
  log_info(logger, "Planificador de Corto Plazo: Ejecutando...");

  while (1) {
    sem_wait(&(COLA_READY->instancias_disponibles));
    enviar_interrupcion();

    t_pcb *pcb = elegir_pcb_segun_algoritmo(COLA_READY);

    cambiar_estado_pcb(pcb, RUNNING), remover_pcb_de_cola(pcb, COLA_READY);

    // TODO: agregar en el planificador de corto plazo
    // esto lanza una excepción si la conexión dispatch de cpu no fue iniciada..
    int socket_cpu_dispatch = conectarse_a_cpu("PUERTO_CPU_DISPATCH");

    if (socket_cpu_dispatch != -1) {
      t_paquete *paquete = paquete_create();
      paquete_add_pcb(paquete, pcb);
      enviar_pcb(socket_cpu_dispatch, paquete);
    }
    close(socket_cpu_dispatch);
  }

  pthread_exit(NULL);
}

// TODO: añadir un logger sólo para este planificador
void *iniciar_largo_plazo() {
  log_info(logger, "Planificador de Largo Plazo: Ejecutando...");

  while (1) {
    sem_wait(&(COLA_NEW->instancias_disponibles));
    sem_wait(&HAY_PROCESOS_ENTRANTES);

    log_info(logger, "plp: pcbs=%d", queue_size(PCBS_PROCESOS_ENTRANTES));

    t_pcb *pcb = (t_pcb *)queue_pop(PCBS_PROCESOS_ENTRANTES);

    transicion_a_new(pcb);

    controlar_grado_multiprogramacion();
    transicion_new_a_ready(pcb);
    // TODO: enviar_solicitud_tabla_paginas(fd_memoria);

    // TODO: contemplar cuando el proceso finaliza, por momento habrán memory leaks
    // pcb_destroy(pcb);
  }

  pthread_exit(NULL);
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

void transicion_a_new(t_pcb *pcb) {
  agregar_pcb_a_cola(pcb, COLA_NEW);

  sem_post(&(COLA_NEW->instancias_disponibles));

  log_info(
    logger, "Se agregó un PCB (pid=%d) a la cola de NEW (cantidad_pcbs=%d)", pcb->pid, list_size(COLA_NEW->lista_pcbs));
}

void transicion_new_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_NEW);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);

  log_info(logger,
           "Se agregó un PCB (pid=%d) a la cola de READY (cantidad_pcbs=%d)",
           pcb->pid,
           list_size(COLA_READY->lista_pcbs));
  sem_post(&(COLA_READY->instancias_disponibles));
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

t_cola_planificacion *cola_planificacion_create() {
  int sem_init_valor = 0;
  t_cola_planificacion *cola = malloc(sizeof(t_cola_planificacion));

  cola->lista_pcbs = list_create();
  pthread_mutex_init(&(cola->mutex), NULL);

  // TODO: evaluar si recibirlo por parámetro o definir una nueva función cola_asignar_instancias(cola, cantidad)
  sem_init(&(cola->instancias_disponibles), 0, sem_init_valor);

  log_info(logger, "Se creó una cola de planificación");

  return cola;
}

void inicializar_grado_multiprogramacion() {
  int grado = atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));

  sem_init(&GRADO_MULTIPROGRAMACION, 0, grado);
}

int obtener_grado_multiprogramacion() {
  int grado;
  sem_getvalue(&GRADO_MULTIPROGRAMACION, &grado);

  return grado;
}

void subir_grado_multiprogramacion() {
  sem_post(&GRADO_MULTIPROGRAMACION);

  log_info(logger, "Subió el grado de multiprogramación (grado=%d)", obtener_grado_multiprogramacion());
}

void controlar_grado_multiprogramacion() {
  sem_wait(&GRADO_MULTIPROGRAMACION);

  log_info(logger, "Bajó el grado de multiprogramación (grado=%d)", obtener_grado_multiprogramacion());
}

t_pcb *elegir_pcb_fifo(t_cola_planificacion *cola) {
  t_pcb *pcb = NULL;

  pthread_mutex_lock(&(cola->mutex));
  pcb = (t_pcb *)list_get(cola->lista_pcbs, 0);
  pthread_mutex_unlock(&(cola->mutex));

  return pcb;
}

t_pcb *elegir_pcb_sjf(t_cola_planificacion *cola) {
  t_pcb *pcb = NULL;

  pthread_mutex_lock(&(cola->mutex));
  pcb =
    (t_pcb *)list_get_minimum(cola->lista_pcbs, (void *)pcb_menor_rafaga_cpu_entre); // si hay empate devuelve por FIFO
  pthread_mutex_unlock(&(cola->mutex));

  return pcb;
}

t_pcb *pcb_menor_rafaga_cpu_entre(t_pcb *pcb1, t_pcb *pcb2) {
  return pcb1->estimacion_rafaga <= pcb2->estimacion_rafaga ? pcb1 : pcb2;
}

t_pcb *elegir_pcb_segun_algoritmo(t_cola_planificacion *cola) {
  t_pcb *pcb = NULL;

  if (algoritmo_cargado_es("FIFO")) {
    pcb = elegir_pcb_fifo(cola);
  }

  else if (algoritmo_cargado_es("SJT")) {
    pcb = elegir_pcb_sjf(cola);
  }

  return pcb;
}

bool algoritmo_cargado_es(char *algoritmo) {
  char *algoritmo_cargado = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

  return strcmp(algoritmo_cargado, algoritmo) == 0;
}

void enviar_interrupcion() {
  t_paquete *paquete = paquete_create();
  paquete->codigo_operacion = INTERRUPT;

  int socket_destino = conectarse_a_cpu("PUERTO_CPU_INTERRUPT");

  if (socket_destino != -1) {
    int status = enviar(socket_destino, paquete);

    if (status != -1) {
      log_info(logger, "La interrupcion fue enviada con éxito (socket_destino=%d)", socket_destino);

      close(socket_destino);
    }
  }
}
