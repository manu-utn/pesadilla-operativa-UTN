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

  logger = iniciar_logger(DIR_LOG_MESSAGES, "Cliente-1");

  config = iniciar_config(DIR_CLIENTE_CFG);
  /* mensaje = config_get_string_value(config, "MENSAJE"); */

  ip = config_get_string_value(config, "IP");
  puerto = config_get_string_value(config, "PUERTO");
  fd_servidor = conectar_a_servidor(ip, puerto);

  t_paquete* paquete1 = NULL;
  t_buffer* mensaje = NULL;
  paquete1 = paquete_create();
  mensaje = crear_mensaje("chau"); // TODO: need free x2
  paquete_cambiar_mensaje(paquete1, mensaje);
  enviar_mensaje(fd_servidor, paquete1);
  paquete_destroy(paquete1);

  // Enviamos otro mensaje
  t_paquete* paquete2 = paquete_create();
  // paquete2->buffer = crear_mensaje("aaaaa");  // <-- NO HACER, usar
  // paquete_cambiar_mensaje()
  paquete_cambiar_mensaje(paquete2, crear_mensaje("punchi punchi"));
  enviar_mensaje(fd_servidor, paquete2);
  paquete_destroy(paquete2);

  // Enviamos un paquete con 2 mensajes
  t_paquete* paquete3 = paquete_create();     // TODO: need free x3
  t_buffer* mensaje1 = crear_mensaje("chau"); // TODO: need free x2
  t_buffer* mensaje2 = crear_mensaje("wi");   // TODO: need free x2

  paquete_add_mensaje(paquete3, mensaje1);
  paquete_add_mensaje(paquete3, mensaje2);

  enviar_paquete(fd_servidor, paquete3);

  mensaje_destroy(mensaje1);
  mensaje_destroy(mensaje2);
  paquete_destroy(paquete3);

  terminar_cliente(fd_servidor, logger, config);

  return 0;
}
