#include "cpu.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <libstatic.h> // <-- STATIC LIB

// void* escuchar_dispatch(void* arguments) {
void* escuchar_dispatch() {
  // struct arg_struct* args = (struct arg_struct*)arguments;
  estado_conexion_kernel = true;

  // memcpy(ip, args->arg1, strlen(args->arg1));
  // memcpy(puerto, args->arg2, strlen(args->arg2));
  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  int socket_cpu_dispatch = iniciar_servidor(ip, puerto);

  while (estado_conexion_kernel) {
    int cliente_fd = esperar_cliente(socket_cpu_dispatch);

    if (cliente_fd != -1) {
      t_paquete* paquete = paquete_create();
      t_buffer* mensaje = crear_mensaje("Conexión aceptada por CPU");

      paquete_cambiar_mensaje(paquete, mensaje), enviar_mensaje(cliente_fd, paquete);
      // paquete_add_mensaje(paquete, mensaje);
    }

    pthread_t th;
    pthread_create(&th, NULL, manejar_nueva_conexion, &cliente_fd), pthread_detach(th);
  }
  free(ip);
  free(puerto);
  pthread_exit(NULL);
}

void* manejar_nueva_conexion(void* args) {
  int socket_cliente = *(int*)args;

  estado_conexion_con_cliente = true;

  while (estado_conexion_con_cliente) {
    int cod_op = recibir_operacion(socket_cliente);

    // MENSAJE=0, PAQUETE=1
    switch (cod_op) {
      case PCB: {
        t_paquete* paquete_con_pcb = malloc(sizeof(t_paquete) + 1);
        paquete_con_pcb = recibir_paquete(socket_cliente);

        t_pcb* pcb_deserializado = paquete_obtener_pcb(paquete_con_pcb);
        pcb_deserializado->socket = socket_cliente;
        ciclo_instruccion(pcb_deserializado);
        imprimir_pcb(pcb_deserializado);
        // pcb_destroy(pcb_deserializado); DESCOMENTAR PARA SACAR EL SEG FAULT
        paquete_destroy(paquete_con_pcb);
        free(pcb_deserializado);

        // descomentar para validar el memcheck
        // terminar_servidor(socket_cpu_dispatch, logger, config);
        // return 0;
      } break;
      case -1: {
        log_info(logger, "el cliente se desconecto");
        // cliente_estado = CLIENTE_EXIT;
        estado_conexion_kernel = false;
        estado_conexion_con_cliente = false;
        break;
      }
      default:
        log_warning(logger, "Operacion desconocida. No quieras meter la pata");
        break;
    }
  }
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

void ciclo_instruccion(t_pcb* pcb) {
  log_info(logger, "Iniciando ciclo de instruccion");
  log_info(logger, "leyendo instrucciones");
  int i = 0;

  while (pcb->program_counter < list_size(pcb->instrucciones)) {
    t_instruccion* instruccion = malloc(sizeof(t_instruccion) + 1);
    instruccion = fetch(pcb);
    decode(instruccion, pcb);
    // free(instruccion);
    pcb->program_counter++;
  }
  pcb_destroy(pcb);
}

t_instruccion* fetch(t_pcb* pcb) {
  return list_get(pcb->instrucciones, pcb->program_counter - 1);
}

void decode(t_instruccion* instruccion, t_pcb* pcb) {
  if (strcmp(instruccion->identificador, "NO_OP") == 0) {
    log_info(logger, "Ejecutando NO_OP...");
    // int retardo = (float)config_get_int_value(config, "RETARDO_NOOP") / (float)1000;
    int retardo = 1;
    sleep(retardo);
  }

  else if (strcmp(instruccion->identificador, "I/O") == 0) {
    log_info(logger, "Ejecutando IO...");
    t_paquete* paquete_con_pcb = paquete_create();
    uint32_t tiempo_bloqueo = 0;
    tiempo_bloqueo = atoi(instruccion->params);
    // paquete_add_operacion_IO(paquete_con_pcb, pcb, tiempo_bloqueo);  //DESCOMENTAR PARA PROBAR LA RESPUESTA A KERNEL
    // enviar_pcb(pcb->socket, paquete_con_pcb);
    paquete_destroy(paquete_con_pcb);
  }

  else if (strcmp(instruccion->identificador, "READ") == 0) {
    log_info(logger, "Ejecutando READ...");

    t_operacion_read* read = malloc(sizeof(t_operacion_read));

    armar_operacion_read(read, instruccion);

    t_paquete* paquete_con_direccion_a_leer = paquete_create();
    read->socket = socket_memoria;
    paquete_add_operacion_read(paquete_con_direccion_a_leer, read);
    enviar_operacion_read(socket_memoria, paquete_con_direccion_a_leer);
    // operacion_read_destroy(pcb);
    free(read);
    paquete_destroy(paquete_con_direccion_a_leer);

    // RECIBO RESPUESTA DE MEMORIA

    t_paquete* paquete_respuesta = recibir_paquete(socket_memoria);

    t_respuesta_operacion_read* respuesta_operacion = obtener_respuesta_read(paquete_respuesta);

    log_info(logger, "RESPUESTA VALOR MEMORIA: %d ", respuesta_operacion->valor_buscado);

    paquete_destroy(paquete_con_direccion_a_leer);
    free(respuesta_operacion);
  }

  else if (strcmp(instruccion->identificador, "WRITE") == 0) {
  }

  else if (strcmp(instruccion->identificador, "COPY") == 0) {
  } else if (strcmp(instruccion->identificador, "EXIT") == 0) {
    t_paquete* paquete_con_respuesta_exit = paquete_create();
    pcb->program_counter++;
    paquete_add_pcb(paquete_con_respuesta_exit, pcb);
    enviar_pcb(pcb->socket, paquete_con_respuesta_exit);
    // pcb_destroy(pcb); // DESCOMENTAR PARA RESOLVER SEG FAULT
    paquete_destroy(paquete_con_respuesta_exit);
  }

  else if (strcmp(instruccion->identificador, "EXIT") == 0) {
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
