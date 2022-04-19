#include "kernel.h"
#include "planificador.h"
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

  // int socket_cpu_dispatch = conectarse_a_cpu_dispatch();

  int socket = iniciar_servidor(ip, puerto);
  log_info(logger, "Servidor listo para recibir al cliente Consola");

  // int SOCKET_CPU;

  while (1) {
    int cliente_fd = esperar_cliente(socket);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      switch (cod_op) {
        case CONSOLA: {
          t_paquete* paquete = recibir_paquete(cliente_fd);

          // deserializamos
          t_list* lista_instrucciones = paquete_obtener_instrucciones(paquete);
          paquete_destroy(paquete);

          list_destroy_and_destroy_elements(lista_instrucciones,
                                            (void*)instruccion_destroy);

          /* t_pcb* pcb = create_pcb(cliente_fd, ULTIMO_PID); */
          /* t_paquete* paquete_con_pcb = paquete_create(); */
          /* asignar_codigo_operacion(PCB, paquete_con_pcb); */
        } break;
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

void enviar_instruccion(int socket, op_code codigo_operacion, char** params) {
  /*
  t_paquete* paquete = paquete_create();

  switch (codigo_operacion) {
    case IO: {
      asignar_codigo_operacion(IO, paquete);
      paquete_add_mensaje(paquete, crear_mensaje(params[1]));
    } break;
    case READ: {
      asignar_codigo_operacion(READ, paquete);
      paquete_add_mensaje(paquete, crear_mensaje(params[1]));
    } break;
    case WRITE: {
      asignar_codigo_operacion(WRITE, paquete);
      paquete_add_mensaje(paquete, crear_mensaje(params[1]));
      paquete_add_mensaje(paquete, crear_mensaje(params[2]));
    } break;
    case COPY: {
      asignar_codigo_operacion(COPY, paquete);
      paquete_add_mensaje(paquete, crear_mensaje(params[1]));
      paquete_add_mensaje(paquete, crear_mensaje(params[2]));
    } break;
    case EXIT: {
      // TODO: definir
    } break;
    default: {
      // TODO: definir
    } break;
  }

  enviar_paquete(socket, paquete);

  paquete_destroy(paquete);
   */
}

int conectarse_a_cpu_dispatch() {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
  int fd_servidor = conectar_a_servidor(ip, puerto);

  return fd_servidor;
}
