#include "memoria.h"

void* reservar_memoria_inicial(int size_memoria_total) {
  void* memoria_total = malloc(size_memoria_total);

  memset(memoria_total, 0, size_memoria_total);

  return memoria_total;
}

void* escuchar_conexiones() {
  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA");

  int socket_kernel = iniciar_servidor(ip, puerto);

  while (1) {
    int cliente_fd = esperar_cliente(socket_kernel);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int codigo_operacion = recibir_operacion(cliente_fd);

      switch (codigo_operacion) {
        case MENSAJE_HANDSHAKE: {
          log_info(logger, "Handshake establecido...");
          // t_paquete* paquete = recibir_paquete(cliente_fd);
          // t_mensaje_handshake_cpu_memoria* mensaje = paquete_obtener_mensaje_handshake(paquete);

        } break;
        case READ: {
          log_info(logger, "Comenzando operacion READ...");
          t_paquete* paquete = recibir_paquete(cliente_fd);
          t_operacion_read* read = paquete_obtener_operacion_read(paquete);
        }
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
