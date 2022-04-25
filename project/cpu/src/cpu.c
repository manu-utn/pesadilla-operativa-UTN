#include "cpu.h"
#include <libstatic.h> // <-- STATIC LIB


// void* escuchar_dispatch(void* arguments) {
void* escuchar_dispatch() {
  // struct arg_struct* args = (struct arg_struct*)arguments;

  // memcpy(ip, args->arg1, strlen(args->arg1));
  // memcpy(puerto, args->arg2, strlen(args->arg2));
  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  int socket_cpu_dispatch = iniciar_servidor(ip, puerto);

  while (1) {
    int cliente_fd = esperar_cliente(socket_cpu_dispatch);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      // MENSAJE=0, PAQUETE=1
      switch (cod_op) {
        case PCB: {
          t_paquete* paquete_con_pcb = recibir_paquete(cliente_fd);

          t_pcb* pcb_deserializado = paquete_obtener_pcb(paquete_con_pcb);
          pcb_deserializado->socket = cliente_fd;
          procesar_intrucciones(pcb_deserializado);
          imprimir_pcb(pcb_deserializado);
          // pcb_destroy(pcb_deserializado); DESCOMENTAR PARA SACAR EL SEG FAULT
          paquete_destroy(paquete_con_pcb);

          // descomentar para validar el memcheck
          // terminar_servidor(socket_cpu_dispatch, logger, config);
          // return 0;
        } break;
        case -1: {
          log_info(logger, "el cliente se desconecto");
          cliente_estado = CLIENTE_EXIT;
          break;
        }
        default:
          log_warning(logger, "Operacion desconocida. No quieras meter la pata");
          break;
      }
    }
  }
  free(ip);
  free(puerto);
  pthread_exit(NULL);
}

void* escuchar_interrupt() {
  log_info(logger, "Iniciando escucha interrupt....");
  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
  int socket_cpu_dispatch = iniciar_servidor(ip, puerto);

  while (1) {
    int cliente_fd = esperar_cliente(socket_cpu_dispatch);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);
      switch (cod_op) {}
    }
  }

  free(ip);
  free(puerto);
}


void procesar_intrucciones(t_pcb* pcb) {
  log_info(logger, "Iniciando ciclo de instruccion");
  log_info(logger, "leyendo instrucciones");
  int i = 0;

  while (i < list_size(pcb->instrucciones)) {
    // int size_instruccion = strlen(((t_instruccion*)list_get(pcb->instrucciones, i))->identificador);
    t_instruccion* instruccion = malloc(sizeof(t_instruccion) + 1);
    //    memcpy(instruccion, (((t_instruccion*)list_get(pcb->instrucciones, i))->identificador), size_instruccion);
    //   instruccion[size_instruccion] = '\0';
    instruccion = list_get(pcb->instrucciones, i);
    if (strcmp(instruccion->identificador, "NO_OP") == 0) {
      log_info(logger, "Ejecutando NO_OP...");
      pcb->program_counter++;
      // int retardo = (float)config_get_int_value(config, "RETARDO_NOOP") / (float)1000;
      int retardo = 1;
      sleep(retardo);
    }

    else if (strcmp(instruccion->identificador, "I/O") == 0) {
      log_info(logger, "Ejecutando IO...");
      t_paquete* paquete_con_pcb = paquete_create();
      paquete_add_pcb(paquete_con_pcb, pcb);
      enviar_pcb(pcb->socket, paquete_con_pcb);
      // pcb_destroy(pcb); DESCOMENTAR PARA RESOLVER SEG FAULT
      paquete_destroy(paquete_con_pcb);
    }

    else if (strcmp(instruccion->identificador, "READ") == 0) {
      log_info(logger, "Ejecutando READ...");

      t_operacion_read* read = malloc(sizeof(t_operacion_read));

      armar_operacion_read(read, instruccion);

      t_paquete* paquete_con_direccion_a_leer = paquete_create();
      paquete_add_operacion_read(paquete_con_direccion_a_leer, read);
      enviar_operacion_read(socket_memoria, paquete_con_direccion_a_leer);
      operacion_read_destroy(pcb);
      free(read);
      paquete_destroy(paquete_con_direccion_a_leer);
    }

    free(instruccion);
    i++;
  }
}

void armar_operacion_read(t_operacion_read* read, t_instruccion* instruccion) {
  read->direccion_logica = atoi(instruccion->params);
}


int conectarse_a_memoria() {
  char* ip = config_get_string_value(config, "IP_MEMORIA");
  char* puerto = config_get_string_value(config, "PUERTO_MEMORIA");
  int fd_servidor = conectar_a_servidor(ip, puerto);

  if (fd_servidor == -1) {
    log_error(
      logger, "No se pudo establecer la conexión con CPU, inicie el servidor con %s e intente nuevamente", puerto);

    return -1;
  }

  return fd_servidor;
}

void iniciar_tlb() {
  tlb = list_create();
}