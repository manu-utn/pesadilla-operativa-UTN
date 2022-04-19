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
  /*
  t_paquete* paquete1 = paquete_create();
  paquete_cambiar_mensaje(paquete1, crear_mensaje("saludar"));
  enviar_mensaje(fd_kernel, paquete1);
  paquete_destroy(paquete1);
   */

  // Enviamos un paquete con 2 mensajes
  t_paquete* paquete = paquete_create();
  // asignar_codigo_operacion(CONSOLA, paquete);

  t_instruccion* instruccion1 = instruccion_create("NO_OP", "3000");
  t_instruccion* instruccion2 = instruccion_create("WRITE", "4 42");
  t_instruccion* instruccion3 = instruccion_create("READ", "9");

  // serializamos
  paquete_add_instruccion(paquete, instruccion1);
  paquete_add_instruccion(paquete, instruccion2);
  paquete_add_instruccion(paquete, instruccion3);

  enviar_instrucciones(fd_kernel, paquete);

  instruccion_destroy(instruccion1);
  instruccion_destroy(instruccion2);
  instruccion_destroy(instruccion3);
  paquete_destroy(paquete);

  terminar_cliente(fd_kernel, logger, config);

  return 0;
}

int conectarse_a_kernel() {
  char* ip = config_get_string_value(config, "IP_KERNEL");
  char* puerto = config_get_string_value(config, "PUERTO_KERNEL");
  int fd_servidor = conectar_a_servidor(ip, puerto);

  return fd_servidor;
}
