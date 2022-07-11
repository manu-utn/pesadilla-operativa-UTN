#include "libstatic.h"
#include "memoria.h"
#include <commons/string.h>

int obtener_cantidad_entradas_por_tabla_por_config() {
  return config_get_int_value(config, "PAGINAS_POR_TABLA");
}

int obtener_tamanio_memoria_por_config() {
  return config_get_int_value(config, "TAM_MEMORIA");
}

int obtener_cantidad_marcos_por_proceso_por_config() {
  return config_get_int_value(config, "TAM_MEMORIA");
}

int obtener_tamanio_pagina_por_config() {
  return config_get_int_value(config, "TAM_PAGINA");
}

char* obtener_algoritmo_reemplazo_por_config() {
  return config_get_string_value(config, "ALGORITMO_REEMPLAZO");
}

bool algoritmo_reemplazo_cargado_es(char* algoritmo) {
  return strcmp(obtener_algoritmo_reemplazo_por_config(), algoritmo) == 0;
}

char* obtener_path_archivos_swap() {
  return config_get_string_value(config, "PATH_SWAP");
}
