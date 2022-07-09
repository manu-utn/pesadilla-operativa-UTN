#include "utils.h"
#include "libstatic.h"
#include "memoria.h"
#include <commons/collections/dictionary.h>

void inicializar_estructuras() {
  estado_conexion_memoria = true;
  logger = iniciar_logger(DIR_LOG_MESSAGES, "MEMORIA");
  config = iniciar_config(DIR_MEMORIA_CFG);

  memoria_principal = malloc(obtener_tamanio_memoria_por_config());

  llenar_memoria_mock();

  tablas_de_paginas_primer_nivel = dictionary_create();
  tablas_de_paginas_segundo_nivel = dictionary_create();
  tabla_marcos = list_create();

  dividir_memoria_principal_en_marcos();
}
