#include "sample.h"
#include "cpu.h"
#include "serializado.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/config.h>
#include <commons/log.h>
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  // función de la biblioteca static
  // saludar();
  cargarConfiguracion();

  // logger = iniciar_logger(DIR_LOG_MESSAGES, "Servidor-1");

  // t_config* config = iniciar_config(DIR_SERVIDOR_CFG);

  int socket_memoria = conectarse_a_memoria();

  // HANDSHAKE CON MEMORIA
  t_mensaje_handshake_cpu_memoria* mensaje_hs =
    mensaje_handshake_create("MENSAJE PRUEBA");

  t_paquete* paquete_con_mensaje = paquete_create();
  paquete_add_mensaje_handshake(paquete_con_mensaje, mensaje_hs);
  enviar_mensaje_handshake(socket_memoria, paquete_con_mensaje);
  free(mensaje_hs);
  paquete_destroy(paquete_con_mensaje);


  char* ip_cpu = "127.0.0.1";
  char* puerto_cpu_dispatch =
    string_itoa(configuracion->puerto_escucha_dispatch);

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
          procesar_intrucciones(pcb_deserializado->instrucciones);
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
  limpiarConfiguracion();
}


/*
int conectarse_a_memoria() {
  char* ip = config_get_string_value(configuracion, "IP_MEMORIA");
  char* puerto = string_itoa(config_get_string_value(configuracion,
"PUERTO_MEMORIA"));
  int fd_servidor = conectar_a_servidor(ip, puerto);

  return fd_servidor;
}*/
