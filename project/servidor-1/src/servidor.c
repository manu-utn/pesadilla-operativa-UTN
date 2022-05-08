#include "servidor.h"
#include "timer.h"
#include "utils-cliente.h"
#include "xlog.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <libstatic.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

t_pcb* PROCESO_EJECUTANDO;
int CONEXION_CPU_DISPATCH;
int CONEXION_CPU_INTERRUPT;
int SOCKET_CLIENTE_DISPATCH;

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "Servidor-1");
  config = iniciar_config(DIR_SERVIDOR_CFG);

  pthread_t th1, th2;
  pthread_create(&th1, NULL, (void*)iniciar_conexion_interrupt, NULL);
  pthread_create(&th2, NULL, (void*)iniciar_conexion_dispatch, NULL);

  // necesario `detach` en vez de `join`, para separarlo del hilo principal `main`
  // y ejecutar las rutinas asociadas al mismo tiempo, caso contrario una bloqueará la otra porque `recv` es bloqueante
  pthread_detach(th1), pthread_detach(th2);

  // terminar_programa(server_fd, logger, config);

  // necesario en vez de `return 0`, caso contrario el hilo main finalizará antes de los hilos detach
  pthread_exit(0);
}

void* iniciar_conexion_interrupt() {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

  CONEXION_CPU_INTERRUPT = iniciar_servidor(ip, puerto); // TODO: evaluar posibilidad de condición de carrera

  xlog(COLOR_CONEXION, "Conexión Interrupt lista con el cliente Kernel");

  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexiones_entrantes_en_interrupt, NULL), pthread_detach(th);

  pthread_exit(NULL);
}

void* escuchar_conexiones_entrantes_en_interrupt() {
  CONEXION_ESTADO estado_conexion_con_cliente = CONEXION_ESCUCHANDO;
  CONEXION_ESTADO ESTADO_CONEXION_INTERRUPT = CONEXION_ESCUCHANDO;

  while (ESTADO_CONEXION_INTERRUPT) {
    int socket_cliente = esperar_cliente(CONEXION_CPU_INTERRUPT);
    estado_conexion_con_cliente = CONEXION_ESCUCHANDO;

    while (estado_conexion_con_cliente) {
      int codigo_operacion = recibir_operacion(socket_cliente);

      switch (codigo_operacion) {
        case OPERACION_INTERRUPT: {
          t_paquete* paquete = recibir_paquete(socket_cliente);
          xlog(COLOR_PAQUETE, "se recibió una Interrupción");

          timer_detener();
          timer_imprimir();
          desalojar_y_enviar_proceso_en_ejecucion();

          paquete_destroy(paquete);
        } break;
        case OPERACION_MENSAJE: {
          recibir_mensaje(socket_cliente);
        } break;
        case OPERACION_EXIT: {
          xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

          log_destroy(logger), close(CONEXION_CPU_INTERRUPT);
          // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
          ESTADO_CONEXION_INTERRUPT = CONEXION_FINALIZADA;
          estado_conexion_con_cliente = CONEXION_FINALIZADA;
        } break;
        case -1: {
          xlog(COLOR_CONEXION, "el cliente se desconecto (socket=%d)", socket_cliente);

          // centinela para detener el loop del hilo asociado a la conexión
          // entrante
          estado_conexion_con_cliente = CONEXION_FINALIZADA;
        } break;
        default: {
          xlog(COLOR_ERROR, "Operacion %d desconocida", codigo_operacion);
        } break;
      }
    }
  }

  pthread_exit(NULL);
}

void desalojar_y_enviar_proceso_en_ejecucion() {
  t_paquete* paquete = paquete_create();
  t_pcb* pcb = PROCESO_EJECUTANDO;
  pcb->tiempo_en_ejecucion = TIMER.tiempo_total; // en microsegundos

  xlog(COLOR_INFO,
       "Se actualizó el tiempo en ejecución de un pcb (pcb=%d, tiempo=%d)",
       pcb->pid,
       pcb->tiempo_en_ejecucion);

  timer_imprimir();
  imprimir_pcb(pcb);
  paquete_add_pcb(paquete, pcb);

  enviar_pcb_desalojado(SOCKET_CLIENTE_DISPATCH, paquete);
  xlog(COLOR_TAREA, "Se ha desalojado un PCB de CPU (pcb=%d)", pcb->pid);
  // pcb_destroy(PROCESO_EJECUTANDO);
}


void* iniciar_conexion_dispatch() {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, "PUERTO_CPU_DISPATCH");

  CONEXION_CPU_DISPATCH = iniciar_servidor(ip, puerto); // TODO: evaluar posibilidad de condición de carrera

  xlog(COLOR_CONEXION, "Conexión Dispatch lista con el cliente Kernel");

  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexiones_entrantes, NULL), pthread_detach(th);

  pthread_exit(NULL);
}

void* escuchar_conexiones_entrantes(void* args) {
  CONEXION_ESTADO estado_conexion_con_cliente = CONEXION_ESCUCHANDO;
  CONEXION_ESTADO ESTADO_CONEXION_DISPATCH = CONEXION_ESCUCHANDO;

  while (ESTADO_CONEXION_DISPATCH) {
    int socket_cliente = esperar_cliente(CONEXION_CPU_DISPATCH);
    SOCKET_CLIENTE_DISPATCH = socket_cliente;

    while (estado_conexion_con_cliente) {
      int codigo_operacion = recibir_operacion(socket_cliente);

      switch (codigo_operacion) {
        case OPERACION_PCB: {
          t_paquete* paquete_con_pcb = recibir_paquete(socket_cliente);

          PROCESO_EJECUTANDO = paquete_obtener_pcb(paquete_con_pcb);
          xlog(COLOR_TAREA, "Ejecutando instrucciones de un proceso (pid=%d)", PROCESO_EJECUTANDO->pid);
          timer_iniciar();
          // imprimir_pcb(PROCESO_EJECUTANDO);

          iniciar_ciclo_de_instruccion(PROCESO_EJECUTANDO);
          // pcb_destroy(pcb_deserializado);
          // paquete_destroy(paquete_con_pcb);
        } break;
        case OPERACION_MENSAJE: {
          recibir_mensaje(socket_cliente);
        } break;
        case OPERACION_EXIT: {
          xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

          log_destroy(logger), close(CONEXION_CPU_DISPATCH);
          // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
          ESTADO_CONEXION_DISPATCH = CONEXION_FINALIZADA;
          estado_conexion_con_cliente = CONEXION_FINALIZADA;
        } break;
        case -1: {
          xlog(COLOR_CONEXION, "el cliente se desconecto (socket=%d)", socket_cliente);

          // centinela para detener el loop del hilo asociado a la conexión
          // entrante
          estado_conexion_con_cliente = CONEXION_FINALIZADA;
        } break;
        default: {
          xlog(COLOR_ERROR, "Operacion %d desconocida", codigo_operacion);
        } break;
      }
    }
  }

  pthread_exit(NULL);
}

void iniciar_ciclo_de_instruccion(t_pcb* pcb) {
  // emulamos fetch+decode+execute todo junto

  t_list* instrucciones = pcb->instrucciones;

  for (int index = 0; index < list_size(instrucciones); index++) {
    t_instruccion* instruccion = list_get(instrucciones, index);

    validar_operacion_io(pcb, instruccion);
    validar_operacion_exit(pcb, instruccion);
  }

  imprimir_pcb(pcb);
}

void validar_operacion_io(t_pcb* pcb, t_instruccion* instruccion) {
  if (es_esta_instruccion(instruccion, "I/O")) {
    t_paquete* paquete = paquete_create();
    paquete_add_pcb(paquete, pcb);
    int tiempo_bloqueado = instruccion_obtener_parametro(instruccion, 0);

    timer_detener();
    timer_imprimir();
    pcb->tiempo_de_bloqueado = tiempo_bloqueado;
    pcb->tiempo_en_ejecucion = TIMER.tiempo_total;

    xlog(COLOR_INFO, "Se actualizó el tiempo de bloqueo de un proceso (pid=%d, tiempo=%d)", pcb->pid, tiempo_bloqueado);
    enviar_pcb_con_operacion_io(SOCKET_CLIENTE_DISPATCH, paquete);
  }
}

int instruccion_obtener_parametro(t_instruccion* instruccion, int numero_parametro) {
  char** parametros = string_split(instruccion->params, " ");
  int valor = atoi(parametros[numero_parametro]);

  string_iterate_lines(parametros, (void*)free);

  return valor;
}

bool es_esta_instruccion(t_instruccion* instruccion, char* identificador) {
  return strcmp(identificador, instruccion->identificador) == 0;
}
