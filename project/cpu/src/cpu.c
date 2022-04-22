#include "cpu.h"
#include <libstatic.h> // <-- STATIC LIB


void procesar_intrucciones(t_list* instrucciones) {
  log_info(logger, "Iniciando ciclo de instruccion");
  log_info(logger, "leyendo instrucciones");
  int i = 0;

  while (list_get(instrucciones, i) != NULL) {
    int size_instruccion = strlen(((t_instruccion*)list_get(instrucciones, i))->identificador);
    char* instruccion = malloc(size_instruccion + 1);
    memcpy(instruccion, (((t_instruccion*)list_get(instrucciones, i))->identificador), size_instruccion);

    if (strcmp(instruccion, "NO_OP")) {
      log_info(logger, "Ejecutando NO_OP...");
      sleep(configuracion->reetardo_noop);
    }

    else {
      log_info(logger, "Ejecutando IO...");
    }

    free(instruccion);
  }

  i++;
}


int conectarse_a_memoria() {
  char* ip = configuracion->ip_memoria;
  char* puerto = string_itoa(configuracion->puerto_memoria);
  int fd_servidor = conectar_a_servidor(ip, puerto);

  return fd_servidor;
}