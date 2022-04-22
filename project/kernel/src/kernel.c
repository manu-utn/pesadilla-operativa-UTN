#include "kernel.h"
#include "libstatic.h"
#include "planificador.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/string.h>
#include <libstatic.h>
#include <stdio.h>
#include <string.h>

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "KERNEL");
  config = iniciar_config(DIR_SERVIDOR_CFG);

  // esto lanza una excepción si la conexión interrupt de cpu no fue iniciada..
  // TODO: se debe usar cuando reciba una IO de CPU
  // enviar_interrupcion();

  iniciar_planificacion();

  // necesario en vez de `return 0`, caso contrario el hilo main finalizará antes de los hilos detach
  pthread_exit(0);
}

int es_esta_instruccion(char* identificador, char** params) {
  return strcmp(identificador, params[0]) == 0;
}

int conectarse_a_cpu(char* conexion_puerto) {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, conexion_puerto);
  int fd_servidor = conectar_a_servidor(ip, puerto);

  if (fd_servidor == -1) {
    log_error(logger,
              "No se pudo establecer la conexión con CPU, inicie el servidor con %s e intente nuevamente",
              conexion_puerto);

    return -1;
  }

  return fd_servidor;
}

void enviar_interrupcion() {
  t_paquete* paquete = paquete_create();
  paquete->codigo_operacion = INTERRUPT;

  int socket_destino = conectarse_a_cpu("PUERTO_CPU_INTERRUPT");

  if (socket_destino != -1) {
    int status = enviar(socket_destino, paquete);

    if (status != -1) {
      log_info(logger, "La interrupcion fue enviada con éxito (socket_destino=%d)", socket_destino);

      close(socket_destino);
    }
  }
}
