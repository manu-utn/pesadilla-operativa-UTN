#include "cpu_1.h"

uint32_t main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "CPU");
  config = iniciar_config(DIR_CPU_CFG);

  t_paquete* paquete = paquete_create();
  t_buffer* mensaje = crear_mensaje("Conexión aceptada por Kernel");
  paquete_cambiar_mensaje(paquete, mensaje);
  enviar_mensaje(socket_memoria, paquete);

  pthread_t th, th2;
  pthread_create(&th, NULL, escuchar_dispatch_, NULL), pthread_detach(th);
  pthread_create(&th2, NULL, iniciar_conexion_interrupt, NULL), pthread_detach(th2);

  xlog(COLOR_INFO, "CPU - Servidor listo para recibir al cliente Kernel");

  pthread_exit(0);
}

void* escuchar_dispatch_() {
  estado_conexion_kernel = true;

  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  socket_cpu_dispatch = iniciar_servidor(ip, puerto);

  while (estado_conexion_kernel) {
    uint32_t cliente_fd = esperar_cliente(socket_cpu_dispatch);

    pthread_t th;
    pthread_create(&th, NULL, manejar_nueva_conexion_, &cliente_fd), pthread_detach(th);
  }

  free(ip);
  free(puerto);

  pthread_exit(NULL);
}

void* manejar_nueva_conexion_(void* args) {
  uint32_t socket_cliente = *(uint32_t*)args;

  estado_conexion_con_cliente = true;

  while (estado_conexion_con_cliente) {
    uint32_t cod_op = recibir_operacion(socket_cliente);

    switch (cod_op) {
      case OPERACION_PCB: {
        xlog(COLOR_PAQUETE, "CPU - Iniciando una nueva operacion pcb");
        t_paquete* paquete_con_pcb = malloc(sizeof(t_paquete) + 1);
        paquete_con_pcb = recibir_paquete(socket_cliente);

        t_pcb* pcb_deserializado = paquete_obtener_pcb(paquete_con_pcb);

        // TODO: hay que pensarlo bien esta variable porque en la teoria esta mal
        HAY_PCB_PARA_EJECUTAR_ = 1;
        ciclo_instruccion(pcb_deserializado, socket_cliente);
        imprimir_pcb(pcb_deserializado);
        paquete_destroy(paquete_con_pcb);
        break;
      }
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

        log_destroy(logger), close(socket_cpu_dispatch);
        // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
        estado_conexion_kernel = false;
        estado_conexion_con_cliente = false;
        break;
      }
      case -1: {
        xlog(COLOR_CONEXION, "el cliente se desconecto");
        // cliente_estado = CLIENTE_EXIT;
        estado_conexion_con_cliente = false;
        break;
      }
      default: {
        xlog(COLOR_ERROR, "Operacion desconocida.");
        break;
      }
    }
  }
  pthread_exit(NULL);
}

void ciclo_instruccion(t_pcb* pcb, uint32_t socket_cliente) {
  xlog(COLOR_INFO, "Iniciando ciclo de instruccion, pcbid: %d", pcb->pid);

  while (HAY_PCB_PARA_EJECUTAR_ && pcb->program_counter < list_size(pcb->instrucciones)) {
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    instruccion = fetch(pcb);
    pcb->program_counter++;

    uint32_t fetch_operands_necesaria = decode(instruccion);

    if (fetch_operands_necesaria == 0) {
      fetch_operands();
    }

    execute(pcb, instruccion, socket_cliente);

    check_interrupt(pcb, socket_cliente);
  }

  xlog(COLOR_INFO, "Finalizando ciclo de instruccion, pcbid: %d", pcb->pid);
}

t_instruccion* fetch(t_pcb* pcb) {
  xlog(COLOR_INFO, "Realizando fetch pcb id: %d", pcb->pid);
  return list_get(pcb->instrucciones, pcb->program_counter);
}

uint32_t decode(t_instruccion* instruccion) {
  xlog(COLOR_INFO, "Realizando decode instruccion: %s", instruccion->identificador);
  return (strcmp(instruccion->identificador, "COPY"));
}

void fetch_operands() {
}

void execute(t_pcb* pcb, t_instruccion* instruccion, uint32_t socket_cliente) {
  xlog(COLOR_INFO, "Realizando execute pcb id: %d", pcb->pid);

  if (strcmp(instruccion->identificador, "NO_OP") == 0) {
    xlog(COLOR_INFO, "Ejecutando NO_OP, pcb id: %d", pcb->pid);
    uint32_t cantidad_de_veces_no_op = instruccion_obtener_parametro(instruccion, 0);
    xlog(COLOR_INFO, "NO_OP se ejecutara %d veces", cantidad_de_veces_no_op);

    execute_no_op(cantidad_de_veces_no_op);

  } else if (strcmp(instruccion->identificador, "I/O") == 0) {
    xlog(COLOR_INFO, "Ejecutando I/O, pcb id: %d", pcb->pid);
    execute_io(pcb, instruccion, socket_cliente);

  } else if (strcmp(instruccion->identificador, "EXIT") == 0) {
    xlog(COLOR_CONEXION, "Ejecutando EXIT, pcb id: %d", pcb->pid);
    execute_exit(pcb, socket_cliente);
  }
}

void check_interrupt(t_pcb* pcb, uint32_t socket_cliente) {
  if (HAY_PCB_PARA_EJECUTAR_) {
    xlog(COLOR_INFO, "Realizando check interrupt");
    if (HAY_INTERRUPCION_) {
      t_paquete* paquete = paquete_create();
      paquete_add_pcb(paquete, pcb);
      enviar_pcb_desalojado(socket_cliente, paquete);
      xlog(COLOR_TAREA, "Interrupcion - Se ha desalojado un PCB de CPU (pcb=%d)", pcb->pid);
      HAY_PCB_PARA_EJECUTAR_ = 0;
      HAY_INTERRUPCION_ = 0;
    }
  } else {
    HAY_INTERRUPCION_ = 0; // Para el caso en el que no haya pcb pero se haya mandado una interrupcion
  }
}

void execute_no_op(uint32_t cant_no_op) {
  uint32_t retardo = config_get_int_value(config, "RETARDO_NOOP");
  xlog(COLOR_INFO, "Retardo de NO_OP en milisegundos: %d", retardo);
  usleep(cant_no_op * retardo * 1000);
}

void execute_io(t_pcb* pcb, t_instruccion* instruccion, uint32_t socket_cliente) {
  uint32_t tiempo_bloqueado = instruccion_obtener_parametro(instruccion, 0);
  pcb->tiempo_de_bloqueado = tiempo_bloqueado;

  t_paquete* paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  xlog(COLOR_INFO, "Se actualizó el tiempo de bloqueo de un proceso (pid=%d, tiempo=%d)", pcb->pid, tiempo_bloqueado);
  enviar_pcb_con_operacion_io(socket_cliente, paquete);
  HAY_PCB_PARA_EJECUTAR_ = 0;
}

void execute_exit(t_pcb* pcb, uint32_t socket_cliente) {
  t_paquete* paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  enviar_pcb_con_operacion_exit(socket_cliente, paquete);
  HAY_PCB_PARA_EJECUTAR_ = 0;
}

uint32_t instruccion_obtener_parametro(t_instruccion* instruccion, uint32_t numero_parametro) {
  char** parametros = string_split(instruccion->params, " ");
  uint32_t valor = atoi(parametros[numero_parametro]);

  string_iterate_lines(parametros, (void*)free);

  return valor;
}

void* iniciar_conexion_interrupt() {
  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

  CONEXION_CPU_INTERRUPT = iniciar_servidor(ip, puerto); // TODO: evaluar posibilidad de condición de carrera

  xlog(COLOR_CONEXION, "Conexión Interrupt lista con el cliente Kernel");

  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexiones_entrantes_en_interrupt, NULL), pthread_detach(th);

  pthread_exit(NULL);
}

void* escuchar_conexiones_entrantes_en_interrupt() {
  CONEXION_ESTADO estado_conexion_con_cliente = CONEXION_ESCUCHANDO;
  CONEXION_ESTADO ESTADO_CONEXION_INTERRUPT = CONEXION_ESCUCHANDO;

  while (ESTADO_CONEXION_INTERRUPT) {
    uint32_t socket_cliente = esperar_cliente(CONEXION_CPU_INTERRUPT);
    estado_conexion_con_cliente = CONEXION_ESCUCHANDO;

    while (estado_conexion_con_cliente) {
      uint32_t codigo_operacion = recibir_operacion(socket_cliente);

      switch (codigo_operacion) {
        case OPERACION_INTERRUPT: {
          t_paquete* paquete = recibir_paquete(socket_cliente);
          xlog(COLOR_PAQUETE, "se recibió una Interrupción");
          /*
          t_paquete* paquete_con_pcb = malloc(sizeof(t_paquete) + 1);
          paquete_con_pcb = recibir_paquete(socket_cliente);
          t_pcb* pcb_deserializado = paquete_obtener_pcb(paquete_con_pcb);

          pcb_deserializado->program_counter++;
          t_paquete* paquete_respuesta = paquete_create();
          t_buffer* mensaje = crear_mensaje_pcb_actualizado(pcb_deserializado, NULL);
          paquete_cambiar_mensaje(paquete_respuesta, mensaje);
          enviar_pcb_interrupt(socket_cliente, paquete_respuesta);
          */
          HAY_INTERRUPCION_ = 1;
          paquete_destroy(paquete);
        } break;
        case OPERACION_MENSAJE: {
          recibir_mensaje(socket_cliente);
        } break;
        case OPERACION_EXIT: {
          xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

          log_destroy(logger), close(CONEXION_CPU_INTERRUPT);
          // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
          ESTADO_CONEXION_INTERRUPT = CONEXION_FINALIZADA;
          estado_conexion_con_cliente = CONEXION_FINALIZADA;
        } break;
        case -1: {
          xlog(COLOR_CONEXION, "el cliente se desconecto (socket=%d)", socket_cliente);

          // centinela para detener el loop del hilo asociado a la conexión
          // entrante
          estado_conexion_con_cliente = CONEXION_FINALIZADA;
        } break;
        default: { xlog(COLOR_ERROR, "Operacion %d desconocida", codigo_operacion); } break;
      }
    }
  }

  pthread_exit(NULL);
}
