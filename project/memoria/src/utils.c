#include "utils.h"
#include "libstatic.h"
#include "memoria.h"
#include <commons/collections/dictionary.h>

void inicializar_estructuras() {
  estado_conexion_memoria = true;
  logger = iniciar_logger(DIR_LOG_MESSAGES, "MEMORIA");
  config = iniciar_config(DIR_MEMORIA_CFG);

  uint32_t size_memoria = config_get_int_value(config, "TAM_MEMORIA");
  memoria_principal = malloc(size_memoria);

  // llenar_memoria_mock();

  ULTIMO_ID_1er_nivel = 0;
  ULTIMO_ID_2do_nivel = 0;

  tablas_de_paginas_primer_nivel = dictionary_create();
  tablas_de_paginas_segundo_nivel = dictionary_create();
  tabla_marcos = list_create();

  dividir_memoria_principal_en_marcos();
}
