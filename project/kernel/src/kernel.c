#include "kernel.h"
#include "libstatic.h"
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

  int socket_cpu_dispatch;

  int socket = iniciar_servidor(ip, puerto);
  log_info(logger, "Servidor listo para recibir al cliente Consola");

  // esto fallará si la conexión cpu_interrupt no está ejecutando
  // enviar_interrupcion();

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

          socket_cpu_dispatch = conectarse_a_cpu("PUERTO_CPU_DISPATCH");
          enviar_pcb(socket_cpu_dispatch, paquete_con_pcb);
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


  return 0;
}

int es_esta_instruccion(char* identificador, char** params) {
  return strcmp(identificador, params[0]) == 0;
}

int conectarse_a_cpu(char* conexion_puerto) {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, conexion_puerto);
  int fd_servidor = conectar_a_servidor(ip, puerto);

  return fd_servidor;
}

void enviar_interrupcion() {
  int socket_destino = conectarse_a_cpu("PUERTO_CPU_INTERRUPT");

  t_paquete* paquete = paquete_create();
  paquete->codigo_operacion = INTERRUPT;

  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    log_info(logger, "La interrupcion fue enviada con éxito (socket_destino=%d)", socket_destino);

    close(socket_destino);
  }
}
