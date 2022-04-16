#include "kernel.h"
#include "planificador.h"
#include "utils-servidor.h"
#include <libstatic.h>
#include <stdio.h>

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "KERNEL");

  t_config* config;
  char* ip;
  char* puerto;

  config = iniciar_config(DIR_SERVIDOR_CFG);
  ip = config_get_string_value(config, "IP_KERNEL");
  puerto = config_get_string_value(config, "PUERTO_KERNEL");

  int socket = iniciar_servidor(ip, puerto);
  log_info(logger, "Servidor listo para recibir al cliente Consola");

  while (1) {
    int cliente_fd = esperar_cliente(socket);
    cliente_status cliente_estado = RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      // MENSAJE=0, PAQUETE=1
      switch (cod_op) {
        case MENSAJE: {
          t_buffer* mensaje = recibir_mensaje(cliente_fd);

          void* stream = ((t_buffer*)mensaje)->stream;
          int size = ((t_buffer*)mensaje)->size;

          log_info(
            logger, "[MENSAJE] (bytes=%d, stream=%s)", size, (char*)stream);

          mensaje_destroy(mensaje);
        } break;
        case PAQUETE: {
          t_paquete* paquete = recibir_paquete(cliente_fd);
          void** mensajes = deserializar_paquete(paquete);

          void** aux = mensajes;

          for (int i = 0; *aux != NULL; aux++, i++) {
            void* stream = ((t_buffer*)mensajes[i])->stream;
            int size = ((t_buffer*)mensajes[i])->size;

            log_info(
              logger, "[MENSAJE] (bytes=%d, stream=%s)", size, (char*)stream);

            mensaje_destroy(mensajes[i]);
          }

          free(mensajes);
          paquete_destroy(paquete);

        } break;
        case -1: {
          log_info(logger, "el cliente se desconecto");
          cliente_estado = EXIT;
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
