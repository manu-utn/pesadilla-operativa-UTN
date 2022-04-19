#include "kernel.h"
#include "planificador.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/string.h>
#include <libstatic.h>
#include <stdio.h>
#include <string.h>

t_config* config;

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "KERNEL");

  config = iniciar_config(DIR_SERVIDOR_CFG);
  char* ip = config_get_string_value(config, "IP_KERNEL");
  char* puerto = config_get_string_value(config, "PUERTO_KERNEL");

  int socket_cpu_dispatch = conectarse_a_cpu_dispatch();

  int socket = iniciar_servidor(ip, puerto);
  log_info(logger, "Servidor listo para recibir al cliente Consola");

  while (1) {
    int cliente_fd = esperar_cliente(socket);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      switch (cod_op) {
        case CONSOLA: {
          t_paquete* paquete_con_instrucciones = recibir_paquete(cliente_fd);

          // TODO: esto es un valor fake, consola debería mandarnos el tamaño
          // del proceso..
          int tamanio_proceso = 500;

          // deserializamos
          t_list* lista_instrucciones =
            paquete_obtener_instrucciones(paquete_con_instrucciones);

          for (int i = 0; i < list_size(lista_instrucciones); i++) {
            t_instruccion* instruccion = list_get(lista_instrucciones, i);
            imprimir_instruccion(instruccion);
          }

          paquete_destroy(paquete_con_instrucciones);

          t_pcb* pcb = pcb_create(cliente_fd, ULTIMO_PID, tamanio_proceso);
          pcb->socket = 4;
          pcb->tamanio = 5000;
          pcb->estimacion_rafaga = 2;
          pcb->program_counter = 1;
          pcb->instrucciones = lista_instrucciones;

          t_paquete* paquete_con_pcb = paquete_create();
          paquete_add_pcb(paquete_con_pcb, pcb);
          enviar_pcb(socket_cpu_dispatch, paquete_con_pcb);

          free(pcb);
          list_destroy_and_destroy_elements(lista_instrucciones,
                                            (void*)instruccion_destroy);
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
          log_warning(logger,
                      "Operacion desconocida. No quieras meter la pata");
          break;
      }
    }
  }


  return 0;
}

int es_esta_instruccion(char* identificador, char** params) {
  return strcmp(identificador, params[0]) == 0;
}

int conectarse_a_cpu_dispatch() {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
  int fd_servidor = conectar_a_servidor(ip, puerto);

  return fd_servidor;
}
