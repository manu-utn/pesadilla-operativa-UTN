#include "planificador.h"

int obtener_tiempo_maximo_bloqueado() {
  return config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");
}

char *obtener_algoritmo_cargado() {
  return config_get_string_value(config, "ALGORITMO_PLANIFICACION");
}

double obtener_alfa_por_config() {
  return config_get_double_value(config, "ALFA");
}

int obtener_grado_multiprogramacion_por_config() {
  return atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));
}
