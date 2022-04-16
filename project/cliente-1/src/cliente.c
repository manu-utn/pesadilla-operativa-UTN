#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <libstatic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cliente.h"

t_config* config;

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "Cliente-1");
  config = iniciar_config(DIR_CLIENTE_CFG);

  int fd_kernel = conectarse_a_kernel();

  // Enviamos otro mensaje
  t_paquete* paquete1 = paquete_create();
  paquete_cambiar_mensaje(paquete1, crear_mensaje("fingo ser una consola"));
  enviar_mensaje(fd_kernel, paquete1);
  paquete_destroy(paquete1);

  // Enviamos un paquete con 2 mensajes
  t_paquete* paquete2 = paquete_create();
  t_buffer* mensaje1 = crear_mensaje("instruccion1");
  t_buffer* mensaje2 = crear_mensaje("instruccion2");

  paquete_add_mensaje(paquete2, mensaje1);
  paquete_add_mensaje(paquete2, mensaje2);

  enviar_paquete(fd_kernel, paquete2);

  mensaje_destroy(mensaje1);
  mensaje_destroy(mensaje2);
  paquete_destroy(paquete2);

  terminar_cliente(fd_kernel, logger, config);

  return 0;
}

int conectarse_a_kernel() {
  char* ip = config_get_string_value(config, "IP_KERNEL");
  char* puerto = config_get_string_value(config, "PUERTO_KERNEL");
  int fd_servidor = conectar_a_servidor(ip, puerto);

  return fd_servidor;
}
