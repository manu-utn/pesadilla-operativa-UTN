#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serializado.h"
#include "servidor.h"
#include <libstatic.h>

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "Servidor-1");

  t_config* config = iniciar_config(DIR_SERVIDOR_CFG);

  char* ip_cpu = config_get_string_value(config, "IP_CPU");
  char* puerto_cpu_dispatch =
    config_get_string_value(config, "PUERTO_CPU_DISPATCH");

  int socket_cpu_dispatch = iniciar_servidor(ip_cpu, puerto_cpu_dispatch);

  log_info(logger, "Servidor listo para recibir al cliente Kernel");

  // TODO: esto debería estar en un hilo, para poder emular e iniciar la
  // también la conexion interrupt
  while (1) {
    int cliente_fd = esperar_cliente(socket_cpu_dispatch);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      // MENSAJE=0, PAQUETE=1
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
          log_warning(logger,
                      "Operacion desconocida. No quieras meter la pata");
          break;
      }
    }
  }


  // terminar_programa(server_fd, logger, config);

  return 0;
}
