#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <libstatic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cliente.h"

t_config* config;

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "Cliente-1");
  config = iniciar_config(DIR_CLIENTE_CFG);

  int fd_kernel = conectarse_a_kernel();

  t_list* lista_instrucciones = list_create();
  list_add(lista_instrucciones, instruccion_create("NO_OP", "3000"));
  list_add(lista_instrucciones, instruccion_create("WRITE", "4 42"));
  list_add(lista_instrucciones, instruccion_create("READ", "9"));

  t_pcb* pcb = pcb_fake();
  pcb->tamanio = 11111; // TODO: debe ser información recibida por la terminal
  pcb->instrucciones = lista_instrucciones; // TODO: debe ser info parseada de un archivo config
  t_paquete* paquete_con_pcb = paquete_create();
  paquete_add_pcb(paquete_con_pcb, pcb);

  enviar_pcb(fd_kernel, paquete_con_pcb);

  paquete_destroy(paquete_con_pcb);

  terminar_cliente(fd_kernel, logger, config);

  return 0;
}

int conectarse_a_kernel() {
  char* ip = config_get_string_value(config, "IP_KERNEL");
  char* puerto = config_get_string_value(config, "PUERTO_KERNEL");
  int fd_servidor = conectar_a_servidor(ip, puerto);

  return fd_servidor;
}
