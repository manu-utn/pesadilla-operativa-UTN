#include "planificador.h"
#include "kernel.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <libstatic.h>
#include <stdio.h>
#include <string.h>

void iniciar_planificacion() {
  pthread_t th;
  ULTIMO_PID = 0;

  COLA_NEW = cola_planificacion_create();
  /* COLA_READY = inicializar_cola(COLA_READY); */
  /* COLA_BLOCKED = inicializar_cola(COLA_BLOCKED); */
  /* COLA_SUSREADY = inicializar_cola(COLA_SUSREADY); */
  /* COLA_SUSBLOCKED = inicializar_cola(COLA_SUSBLOCKED); */

  pthread_create(&th, NULL, iniciar_largo_plazo, NULL);
  pthread_detach(th);

  // TODO: validar cuando debemos liberar los recursos asignados a las colas de planificación
  // cola_destroy(COLA_NEW);
}

// TODO: validar agregando PCBs con instrucciones
void cola_destroy(t_cola_planificacion *cola) {
  list_destroy_and_destroy_elements(cola->lista_pcbs, (void *)pcb_destroy);
  free(cola);
}

// TODO: definir
void *iniciar_corto_plazo() {
  return NULL;
}

// TODO: añadir un logger sólo para este planificador
void *iniciar_largo_plazo() {
  char *ip = config_get_string_value(config, "IP_KERNEL");
  char *puerto = config_get_string_value(config, "PUERTO_KERNEL");

  int socket = iniciar_servidor(ip, puerto);
  log_info(logger, "Planificador de Largo Plazo: Ejecutando...");

  while (1) {
    int cliente_fd = esperar_cliente(socket);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      switch (cod_op) {
        case PCB: {
          t_paquete *paquete_con_pcb = recibir_paquete(cliente_fd);

          t_pcb *pcb_deserializado = paquete_obtener_pcb(paquete_con_pcb);
          imprimir_pcb(pcb_deserializado);

          transicion_a_new(pcb_deserializado);

          // TODO: añadir cuando tengas el feature del grado de multiprogramación
          // sem_wait(&PROCESOS_PENDIENTES_A_INGRESAR);

          // esto lanza una excepción si la conexión dispatch de cpu no fue iniciada..
          int socket_cpu_dispatch = conectarse_a_cpu("PUERTO_CPU_DISPATCH");

          if (socket_cpu_dispatch != -1) {
            enviar_pcb(socket_cpu_dispatch, paquete_con_pcb);
          }
          close(socket_cpu_dispatch);

          pcb_destroy(pcb_deserializado);
          paquete_destroy(paquete_con_pcb);

          // descomentar para validar el memcheck
          // terminar_servidor(socket, logger, config);
          // return 0;
        } break;
        case -1: {
          log_info(logger, "el cliente se desconecto");
          cliente_estado = CLIENTE_EXIT;
          break;
        }
        default:
          log_warning(logger, "Operacion desconocida. No quieras meter la pata");
          break;
      }
    }
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

  log_info(
    logger, "Se agregó un PCB (pid=%d) a la cola de NEW (cantidad_pcbs=%d)", pcb->pid, list_size(COLA_NEW->lista_pcbs));
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
  // int sem_init_valor = 0;
  t_cola_planificacion *cola = malloc(sizeof(t_cola_planificacion));

  cola->lista_pcbs = list_create();
  pthread_mutex_init(&(cola->mutex), NULL);

  // TODO: evaluar si recibirlo por parámetro o definir una nueva función cola_asignar_instancias(cola, cantidad)
  // sem_init(&(cola->instancias_disponibles), 0, sem_init_valor);

  log_info(logger, "Se creó una cola de planificación");

  return cola;
}

// Comentamos código que no fue probado,
// para evitar arrojar errores en el planificador

t_pcb *select_pcb_by_algorithm(t_cola_planificacion *cola, algoritmo_planif algoritmo) {
  pthread_mutex_lock(&(cola->mutex));
  t_pcb *selected_pcb;
  switch (algoritmo) {
    case FIFO:
      selected_pcb = (t_pcb *)list_get(cola->lista_pcbs, 0);
      break;
    case SRT:
      selected_pcb = (t_pcb *)list_get_minimum(cola->lista_pcbs,
                                               (void *)minimum_estimacion_rafaga); // si hay empate devuelve por FIFO
      break;
    default:
      break;
  }
  pthread_mutex_unlock(&(cola->mutex));
  remover_pcb_de_cola(selected_pcb, cola);
  return selected_pcb;
}
/* Solucionado con lo de arriba
t_pcb *select_pcb_by_fifo(t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));
  t_pcb *primer_pcb = (t_pcb *)list_get(cola->lista_pcbs, 0);
  pthread_mutex_unlock(&(cola->mutex));
  remover_pcb_de_cola(primer_pcb, cola);
  return primer_pcb;
}

t_pcb *select_pcb_by_srt(t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));
  t_pcb *srt_pcb =
    (t_pcb *)list_get_minimum(cola->lista_pcbs, (void *)minimum_estimacion_rafaga); // si hay empate devuelve por FIFO
  pthread_mutex_unlock(&(cola->mutex));
  remover_pcb_de_cola(srt_pcb, cola);
  return srt_pcb;
}
*/
t_pcb *minimum_estimacion_rafaga(t_pcb *pcb1, t_pcb *pcb2) {
  return pcb1->estimacion_rafaga <= pcb2->estimacion_rafaga ? pcb1 : pcb2;
}

void cola_destroy(t_cola_planificacion *cola) {
  list_destroy_and_destroy_elements(cola->lista_pcbs, (void *)pcb_destroy);
  free(cola);
}
