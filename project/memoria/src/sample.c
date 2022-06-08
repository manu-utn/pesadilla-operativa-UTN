#include "sample.h"
#include "memoria.h"
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>

int main() {
  // función de la biblioteca static
  estado_conexion_memoria = true;
  logger = iniciar_logger(DIR_LOG_MESSAGES, "MEMORIA");
  config = iniciar_config(DIR_MEMORIA_CFG);
  uint32_t size_memoria = config_get_int_value(config, "TAM_MEMORIA");
  memoria_principal = reservar_memoria_inicial(size_memoria);
  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexiones, NULL), pthread_detach(th);

  pthread_exit(0);
}
