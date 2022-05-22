#include "cpu.h"
/*
int configValida(t_config* fd_configuracion) {
  return (
    config_has_property(fd_configuracion, "IP_ESCUCHA") && config_has_property(fd_configuracion, "ENTRADAS_TLB") &&
    config_has_property(fd_configuracion, "REEMPLAZO_TLB") && config_has_property(fd_configuracion, "RETARDO_NOOP") &&
    config_has_property(fd_configuracion, "IP_MEMORIA") && config_has_property(fd_configuracion, "PUERTO_MEMORIA") &&
    config_has_property(fd_configuracion, "PUERTO_ESCUCHA_DISPATCH") &&
    config_has_property(fd_configuracion, "PUERTO_ESCUCHA_INTERRUPT"));
}


int cargarConfiguracion() {
  logger = log_create("LogCPU", "CPU", false, LOG_LEVEL_INFO);
  configuracion = malloc(sizeof(t_configuracion));

  // en eclipse cambia el path desde donde se corre, asi que probamos desde
  // /Debug y desde /Planificador
  fd_configuracion = config_create(DIR_CPU_CFG);
  if (fd_configuracion == NULL) {
    fd_configuracion = config_create("config.cfg");
  }

  if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
    log_error(logger, "Archivo de configuración inválido.", "ERROR");
    return -1;
  }

  configuracion->ip_escucha = config_get_string_value(fd_configuracion, "IP_ESCUCHA");
  configuracion->entradas_tlb = config_get_int_value(fd_configuracion, "ENTRADAS_TLB");
  configuracion->reemplazo_tlb = config_get_string_value(fd_configuracion, "REEMPLAZO_TLB");
  configuracion->reetardo_noop = config_get_int_value(fd_configuracion, "RETARDO_NOOP");
  configuracion->ip_memoria = config_get_string_value(fd_configuracion, "IP_MEMORIA");
  configuracion->puerto_memoria = config_get_int_value(fd_configuracion, "PUERTO_MEMORIA");
  configuracion->puerto_escucha_dispatch = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA_DISPATCH");
  configuracion->puerto_escucha_interrupt = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA_INTERRUPT");


  log_info(logger,
           "IP_ESCUCHA: %s\n"
           "ENTRADAS_TLB: %d\n"
           "REEMPLAZO_TLB: %s\n"
           "RETARDO_NOOP: %d\n"
           "IP_MEMORIA: %s\n"
           "PUERTO_MEMORIA: %d\n"
           "PUERTO_ESCUCHA_DISPATCH: %d\n"
           "PUERTO_ESCUCHA_INTERRUPT: %d\n",
           configuracion->ip_escucha,
           configuracion->entradas_tlb,
           configuracion->reemplazo_tlb,
           configuracion->reetardo_noop,
           configuracion->ip_memoria,
           configuracion->puerto_memoria,
           configuracion->puerto_escucha_dispatch,
           configuracion->puerto_escucha_interrupt);
  return 0;
}

void limpiarConfiguracion() {
  free(configuracion);
  config_destroy(fd_configuracion);
  log_destroy(logger);
}*/