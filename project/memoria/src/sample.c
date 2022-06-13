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
  size_memoria_principal = config_get_int_value(config, "TAM_MEMORIA");
  llenar_memoria_mock();
  tam_marcos = config_get_int_value(config, "TAM_PAGINA");
  diccionario_paginas = dictionary_create();
  tabla_marcos = list_create();
  lista_tablas_segundo_nivel = list_create();
  xlog(COLOR_CONEXION, "Tamanio tabla de marcos: %d:", inicializar_tabla_marcos());
  // mostrar_tabla_marcos();
  mem_hexdump(memoria_principal, size_memoria_principal);
  inicializar_proceso(0, 4);

  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexiones, NULL), pthread_detach(th);

  pthread_exit(0);
}
