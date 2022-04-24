#include "memoria.h"

int configValida(t_config* fd_configuracion) {
  return (config_has_property(fd_configuracion, "PUERTO_ESCUCHA") &&
          config_has_property(fd_configuracion, "TAM_MEMORIA") && config_has_property(fd_configuracion, "TAM_PAGINA") &&
          config_has_property(fd_configuracion, "PAGINAS_POR_TABLA") &&
          config_has_property(fd_configuracion, "RETARDO_MEMORIA") &&
          config_has_property(fd_configuracion, "ALGORITMO_REEMPLAZO") &&
          config_has_property(fd_configuracion, "MARCOS_POR_PROCESO") &&
          config_has_property(fd_configuracion, "RETARDO_SWAP") && config_has_property(fd_configuracion, "PATH_SWAP"));
}


int cargarConfiguracion() {
  logger = log_create("LogMEMORIA", "CPU", false, LOG_LEVEL_INFO);
  configuracion = malloc(sizeof(t_configuracion));

  // en eclipse cambia el path desde donde se corre, asi que probamos desde
  // /Debug y desde /Planificador
  fd_configuracion = config_create("/home/utnso/tp-2022-1c-Sisop-Oh-Yeah/project/memoria/src/config.cfg");
  if (fd_configuracion == NULL) {
    fd_configuracion = config_create("config.cfg");
  }

  if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
    log_error(logger, "Archivo de configuración inválido.", "ERROR");
    return -1;
  }

  configuracion->puerto_escucha = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA");
  configuracion->tam_memoria = config_get_int_value(fd_configuracion, "TAM_MEMORIA");
  configuracion->tam_pagina = config_get_int_value(fd_configuracion, "TAM_PAGINA");
  configuracion->paginas_por_tabla = config_get_int_value(fd_configuracion, "PAGINAS_POR_TABLA");
  configuracion->retardo_memoria = config_get_int_value(fd_configuracion, "RETARDO_MEMORIA");
  configuracion->algoritmo_reemplazo = config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO");
  configuracion->marcos_por_proceso = config_get_int_value(fd_configuracion, "MARCOS_POR_PROCESO");
  configuracion->retardo_swap = config_get_int_value(fd_configuracion, "RETARDO_SWAP");
  configuracion->path_swap = config_get_int_value(fd_configuracion, "PATH_SWAP");


  log_info(logger,
           "PUERTO_ESCUCHA: %d\n"
           "TAM_MEMORIA: %d\n"
           "TAM_PAGINA: %d\n"
           "PAGINAS_POR_TABLA: %d\n"
           "RETARDO_MEMORIA: %d\n"
           "ALGORITMO_REEMPLAZO: %s\n"
           "MARCOS_POR_PROCESO: %d\n"
           "RETARDO_SWAP: %d\n"
           "PATH_SWAP: %s\n",
           configuracion->puerto_escucha,
           configuracion->tam_memoria,
           configuracion->tam_pagina,
           configuracion->paginas_por_tabla,
           configuracion->retardo_memoria,
           configuracion->algoritmo_reemplazo,
           configuracion->marcos_por_proceso,
           configuracion->retardo_swap,
           configuracion->path_swap);
  return 0;
}

void limpiarConfiguracion() {
  free(configuracion);
  config_destroy(fd_configuracion);
  log_destroy(logger);
}