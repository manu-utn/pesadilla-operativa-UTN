#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libstatic.h"
#include "servidor.h"

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "Servidor-1");

  char* ip;
  char* puerto;

  t_config* config;

  config = iniciar_config(DIR_SERVIDOR_CFG);
  ip = config_get_string_value(config, "IP");
  puerto = config_get_string_value(config, "PUERTO");

  int server_fd = iniciar_servidor(ip, puerto);
  log_info(logger, "Servidor listo para recibir al cliente");
  int cliente_fd = esperar_cliente(server_fd);

  while (1) {
    int cod_op = recibir_operacion(cliente_fd);

    // MENSAJE=0, PAQUETE=1
    switch (cod_op) {
      case MENSAJE: {
        t_buffer* mensaje = recibir_mensaje(cliente_fd); // TODO: need free x2

        void* stream = ((t_buffer*)mensaje)->stream;
        int size = ((t_buffer*)mensaje)->size;

        log_info(
          logger, "[MENSAJE] (bytes=%d, stream=%s)", size, (char*)stream);

        mensaje_destroy(mensaje);
      } break;
      case PAQUETE: {
        t_paquete* paquete = recibir_paquete(cliente_fd); // TODO: need free x3
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
      case -1:
        log_error(logger, "el cliente se desconecto. Terminando servidor");
        return EXIT_FAILURE;
      default:
        log_warning(logger, "Operacion desconocida. No quieras meter la pata");
        break;
    }
  }

  terminar_programa(server_fd, logger, config);

  return 0;
}
