#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <libshared.h> // <-- SHARED LIB
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cliente.h"

int main() {
  int fd_servidor;
  char* ip;
  char* puerto;
  /* char* mensaje; */

  t_config* config;

  logger = log_create(DIR_LOG_MESSAGES, "Cliente-1", 1, LOG_LEVEL_INFO);

  config = iniciar_config(DIR_CLIENTE_CFG);
  /* mensaje = config_get_string_value(config, "MENSAJE"); */

  ip = config_get_string_value(config, "IP");
  puerto = config_get_string_value(config, "PUERTO");
  fd_servidor = conectar_a_servidor(ip, puerto);

  /* t_paquete *paquete = paquete_create(); */
  /* paquete->buffer = crear_mensaje("hola"); // TODO: need free (2) -> y otro
   * free en buffer->stream */
  /* enviar_mensaje(fd_servidor, paquete); */
  /* paquete_destroy(paquete); */

  t_paquete* paquete2 = paquete_create();
  t_buffer* mensaje1 = crear_mensaje("chau");
  t_buffer* mensaje2 = crear_mensaje("wi");

  paquete_add_mensaje(paquete2, mensaje1);
  paquete_add_mensaje(paquete2, mensaje2);

  enviar_paquete(fd_servidor, paquete2);

  mensaje_destroy(mensaje1);
  mensaje_destroy(mensaje2);
  paquete_destroy(paquete2);
  terminar_programa(fd_servidor, logger, config);
  return 0;
}


void terminar_programa(int conexion, t_log* logger, t_config* config) {
  log_destroy(logger), config_destroy(config), liberar_conexion(conexion);
}
