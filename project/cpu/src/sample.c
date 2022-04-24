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
  // cargarConfiguracion();
  logger = iniciar_logger(DIR_LOG_MESSAGES, "CPU");
  config = iniciar_config(DIR_CPU_CFG);

  pthread_t th1, th2;
  struct arg_struct args_dispatch;
  struct arg_struct args_interrupt;
  // logger = iniciar_logger(DIR_LOG_MESSAGES, "Servidor-1");

  // t_config* config = iniciar_config(DIR_SERVIDOR_CFG);

  int socket_memoria = conectarse_a_memoria();

  // HANDSHAKE CON MEMORIA
  t_mensaje_handshake_cpu_memoria* mensaje_hs = mensaje_handshake_create("MENSAJE PRUEBA");

  t_paquete* paquete_con_mensaje = paquete_create();
  paquete_add_mensaje_handshake(paquete_con_mensaje, mensaje_hs);
  enviar_mensaje_handshake(socket_memoria, paquete_con_mensaje);
  free(mensaje_hs);
  paquete_destroy(paquete_con_mensaje);


  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");

  escuchar_dispatch(ip, puerto);

  log_info(logger, "Servidor listo para recibir al cliente Kernel");

  free(config);

  // TODO: esto debería estar en un hilo, para poder emular e iniciar la
  // también la conexion interrupt
}
