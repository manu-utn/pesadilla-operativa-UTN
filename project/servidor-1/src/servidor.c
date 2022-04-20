#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serializado.h"
#include "servidor.h"
#include <libstatic.h>
#include <pthread.h>

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
  int socket = iniciar_servidor(ip, puerto);

  log_info(logger, "Conexion Interrupt lista para recibir al cliente Kernel");

  while (1) {
    int cliente_fd = esperar_cliente(socket);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      switch (cod_op) {
        case INTERRUPT: {
          t_paquete* paquete = recibir_paquete(cliente_fd);
          log_info(logger, "se recibió una interrupción");
          paquete_destroy(paquete);
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

void* iniciar_conexion_dispatch() {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
  int socket = iniciar_servidor(ip, puerto);

  log_info(logger, "Conexion Dispatch lista para recibir al cliente Kernel");

  while (1) {
    int cliente_fd = esperar_cliente(socket);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      switch (cod_op) {
        case PCB: {
          t_paquete* paquete_con_pcb = recibir_paquete(cliente_fd);

          t_pcb* pcb_deserializado = paquete_obtener_pcb(paquete_con_pcb);
          imprimir_pcb(pcb_deserializado);

          pcb_destroy(pcb_deserializado);
          paquete_destroy(paquete_con_pcb);

          // descomentar para validar el memcheck
          // terminar_servidor(socket_cpu_dispatch, logger, config);
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
}
