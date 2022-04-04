#include <commons/collections/list.h>
#include <commons/log.h>
#include <libshared.h> // <-- SHARED LIB
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cliente.h"

int main() {
  int conexion;
  char* ip;
  char* puerto;
  char* valor;

  t_config* config;

  logger = log_create("/home/jelou/Documents/git/manu-cproject/project/"
                      "cliente-1/logs/messages.log",
                      "Cliente",
                      1,
                      LOG_LEVEL_INFO);

  log_info(logger, "Hola! Soy un log");
  config = iniciar_config("/home/jelou/Documents/git/manu-cproject/project/"
                          "cliente-1/config/cliente.cfg");
  valor = config_get_string_value(config, "MENSAJE");

  log_info(logger, valor);

  ip = config_get_string_value(config, "IP");
  puerto = config_get_string_value(config, "PUERTO");
  conexion = conectar_a_servidor(ip, puerto);

  enviar_mensaje(valor, conexion);
  paquete(conexion);

  terminar_programa(conexion, logger, config);
  return 0;
}

void leer_consola(t_log* logger) {
  char* leido = leido = readline(">");

  while (strncmp(leido, "", 1) != 0) {
    log_info(logger, leido);
    free(leido);
    leido = readline(">");
  }

  free(leido);
}

void paquete(int conexion) {
  t_paquete* paquete = crear_paquete();
  char* leido = readline(">");

  while (strncmp(leido, "", 1) != 0) {
    agregar_a_paquete(paquete, leido, strlen(leido) + 1);
    free(leido);
    leido = readline(">");
  }

  free(leido);
  enviar_paquete(paquete, conexion);
  eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log* logger, t_config* config) {
  log_destroy(logger), config_destroy(config), liberar_conexion(conexion);
}
