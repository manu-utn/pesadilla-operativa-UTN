#include "planificador.h"

int obtener_estimacion_inicial_por_config() {
  return config_get_int_value(config, "ESTIMACION_INICIAL");
}

int obtener_tiempo_maximo_bloqueado() {
  return config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");
}

char* obtener_algoritmo_cargado() {
  return config_get_string_value(config, "ALGORITMO_PLANIFICACION");
}

double obtener_alfa_por_config() {
  return config_get_double_value(config, "ALFA");
}

int obtener_grado_multiprogramacion_por_config() {
  return atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));
}

// TODO: pasar a la static lib
char* obtener_ip_de_modulo_por_config(char* nombreDelModulo) {
  char ip[50] = "IP_";
  strcat(ip, nombreDelModulo);

  return config_get_string_value(config, ip);
}

// TODO: pasar a la static lib
char* obtener_puerto_de_modulo_por_config(char* nombreDelModulo) {
  char puerto[50] = "PUERTO_";
  strcat(puerto, nombreDelModulo);

  return config_get_string_value(config, puerto);
}
