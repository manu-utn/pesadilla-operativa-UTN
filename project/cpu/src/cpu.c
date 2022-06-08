#include "cpu.h"
#include "mmu.h"

int main() {
  HAY_PCB_PARA_EJECUTAR_ = 0;
  HAY_INTERRUPCION_ = 0;

  logger = iniciar_logger(DIR_LOG_MESSAGES, "CPU");
  config = iniciar_config(DIR_CPU_CFG);

  // estado_conexion_con_cliente = false;
  // realizar_handshake_memoria();

  pthread_t th, th1, th2;
  pthread_create(&th, NULL, escuchar_dispatch_, NULL), pthread_detach(th);
  pthread_create(&th1, NULL, realizar_handshake_memoria, NULL), pthread_detach(th1);
  pthread_create(&th2, NULL, iniciar_conexion_interrupt, NULL), pthread_detach(th2);

  xlog(COLOR_INFO, "CPU - Servidor listo para recibir al cliente Kernel");

  pthread_exit(0);
}

void realizar_handshake_memoria() {
  xlog(COLOR_INFO, "Inicio handshake con memoria");

  t_paquete* paquete = paquete_create();
  t_buffer* buffer = crear_mensaje("Handshake CPU - MEMORIA");
  paquete->buffer = buffer;

  int socket_memoria = conectarse_a_memoria();
  enviar_mensaje_handshake(socket_memoria, paquete);

  int cliente_fd = esperar_cliente(socket_memoria);

  int cod_op = recibir_operacion(socket_memoria);

  switch (cod_op) {
    case MENSAJE_HANDSHAKE: {
      xlog(COLOR_CONEXION, "Handshake memoria - se recibio la respuesta");
      t_paquete* paquete_respuesta = malloc(sizeof(t_paquete) + 1);
      paquete_respuesta = recibir_paquete(socket_memoria);
      t_mensaje_handshake_cpu_memoria* handshake_deserializado = paquete_obtener_mensaje_handshake(paquete_respuesta);
      tamanio_pagina = handshake_deserializado->tamanio_pagina;
      entradas_por_tabla = handshake_deserializado->entradas_por_tabla;
      xlog(COLOR_INFO,
           "Handshake memoria - respuesta -> entradas por tabla: %d, tamanio pagina: %d",
           handshake_deserializado->entradas_por_tabla,
           handshake_deserializado->tamanio_pagina);
      paquete_destroy(paquete_respuesta);
      break;
    }
    case -1: {
      xlog(COLOR_CONEXION, "Handshake memoria - el cliente se desconecto");
      // cliente_estado = CLIENTE_EXIT;
      estado_conexion_con_cliente = false;
      break;
    }
    default: {
      xlog(COLOR_ERROR, "Operacion desconocida.");
      break;
    }
  }

  paquete_destroy(paquete);

  pthread_exit(NULL);
}

int conectarse_a_memoria() {
  char* ip = config_get_string_value(config, "IP_MEMORIA");
  char* puerto = config_get_string_value(config, "PUERTO_MEMORIA");
  int fd_servidor = conectar_a_servidor(ip, puerto);

  if (fd_servidor == -1) {
    log_error(
      logger, "No se pudo establecer la conexión con MEMORIA, inicie el servidor con %s e intente nuevamente", puerto);

    return -1;
  } else {
    xlog(COLOR_CONEXION, "Se conectó con éxito a Memoria a través de la conexión %s", puerto);
  }

  free(ip);
  free(puerto);

  return fd_servidor;
}

void* escuchar_dispatch_() {
  estado_conexion_kernel = true;

  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  socket_cpu_dispatch = iniciar_servidor(ip, puerto);

  while (estado_conexion_kernel) {
    int cliente_fd = esperar_cliente(socket_cpu_dispatch);

    pthread_t th;
    pthread_create(&th, NULL, manejar_nueva_conexion_, &cliente_fd), pthread_detach(th);
  }

  free(ip);
  free(puerto);

  pthread_exit(NULL);
}

void* manejar_nueva_conexion_(void* args) {
  int socket_cliente = *(int*)args;

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

  // else if (strcmp(instruccion->identificador, "WRITE") == 0) {
  //   log_info(logger, "Ejecutando WRITE...");
  //   int tam_pagina = 64; // TODO: ESTE NUMERO LO TIENE QUE TRAER DE MEMORIA. USAR SOLO PARA PRUEBAS
  //   int cant_entradas_por_tabla = 10;
  //   char** params = string_split(instruccion->params, " ");
  //   uint32_t dir_logica = atoi(params[0]);
  //   void* valor = params[1];
  //   int num_pagina = (float)dir_logica / tam_pagina;
  //   // uint32_t dir_logica = atoi(instruccion->params);
  //   execute_read_write(pcb, tam_pagina, cant_entradas_por_tabla, num_pagina, dir_logica, valor);
  //   free(params);
  //   free(valor);
  // }

  // else if (strcmp(instruccion->identificador, "COPY") == 0) {
  //   xlog(COLOR_CONEXION, "Ejecutando COPY");
  //   int tam_pagina = 64; // TODO: ESTE NUMERO LO TIENE QUE TRAER DE MEMORIA. USAR SOLO PARA PRUEBAS
  //   int cant_entradas_por_tabla = 10;
  //   char** params = string_split(instruccion->params, " ");
  //   uint32_t dir_logica_destino = atoi(params[0]);
  //   uint32_t dir_logica_origen = atoi(params[1]);
  //   int num_pagina = (float)dir_logica_origen / tam_pagina;
  //   t_operacion_respuesta_fetch_operands* respuesta_fetch =
  //     fetch_operands(pcb, tam_pagina, cant_entradas_por_tabla, num_pagina, dir_logica_origen);
  //   execute_read_write(pcb, tam_pagina, cant_entradas_por_tabla, num_pagina, dir_logica_origen,
  //   respuesta_fetch->valor);
  // }
  if (strcmp(instruccion->identificador, "NO_OP") == 0) {
    xlog(COLOR_INFO, "Ejecutando NO_OP, pcb id: %d", pcb->pid);
    uint32_t cantidad_de_veces_no_op = instruccion_obtener_parametro(instruccion, 0);
    xlog(COLOR_INFO, "NO_OP se ejecutara %d veces", cantidad_de_veces_no_op);

    execute_no_op();

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

void execute_no_op() {
  int retardo = config_get_int_value(config, "RETARDO_NOOP");
  xlog(COLOR_INFO, "Retardo de NO_OP en milisegundos: %d", retardo);
  usleep(retardo * 1000);
}

void execute_io(t_pcb* pcb, t_instruccion* instruccion, uint32_t socket_cliente) {
  uint32_t tiempo_bloqueado = instruccion_obtener_parametro(instruccion, 0);
  pcb->tiempo_de_bloqueado = tiempo_bloqueado;

  t_paquete* paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  xlog(COLOR_INFO, "Se actualizó el tiempo de bloqueo de un proceso (pid=%d, tiempo=%d)", pcb->pid, tiempo_bloqueado);
  enviar_pcb_con_operacion_io(socket_cliente, paquete);
}

void execute_exit(t_pcb* pcb, int socket_cliente) {
  t_paquete* paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  enviar_pcb_con_operacion_exit(socket_cliente, paquete);
  HAY_PCB_PARA_EJECUTAR_ = 0;
}

// t_operacion_respuesta_fetch_operands* fetch_operands(t_pcb* pcb,
//                                                      int tam_pagina,
//                                                      int cant_entradas_por_tabla,
//                                                      int num_pagina,
//                                                      uint32_t dir_logica) {
//   log_info(logger, "La pagina no se ecnuentra en la TLB, enviando solicitud a Memoria");
//   int cod_op = 0;

//   // ACCESOS A MEMORIA PARA OBTENER EL MARCO
//   // ACCESO PARA OBTENER TABLA SEGUNDO NIVEL
//   t_solicitud_segunda_tabla* read = malloc(sizeof(t_solicitud_segunda_tabla));
//   obtener_numero_tabla_segundo_nivel(read, pcb, num_pagina, cant_entradas_por_tabla);
//   free(read);


//   // RECIBO RESPUESTA DE MEMORIA
//   xlog(COLOR_INFO, "Recibiendo respuesta de tabla de segundo nivel desde Memoria ");
//   cod_op = recibir_operacion(socket_memoria);
//   t_paquete* paquete_respuesta = recibir_paquete(socket_memoria);
//   t_respuesta_solicitud_segunda_tabla* respuesta_operacion = malloc(sizeof(t_respuesta_solicitud_segunda_tabla));
//   respuesta_operacion = obtener_respuesta_solicitud_tabla_segundo_nivel(paquete_respuesta);
//   printf("Tabla segundo nivel: %d\n", respuesta_operacion->num_tabla_segundo_nivel);

//   // ACCESO PARA OBTENER MARCO
//   t_solicitud_marco* read_marco = malloc(sizeof(t_solicitud_marco));
//   obtener_numero_marco(read_marco, num_pagina, cant_entradas_por_tabla,
//   respuesta_operacion->num_tabla_segundo_nivel);
//   free(read_marco);

//   // RECIBO RESPUESTA DE MEMORIA
//   xlog(COLOR_INFO, "Recibiendo marco nivel desde Memoria ");
//   cod_op = recibir_operacion(socket_memoria);
//   t_paquete* paquete_respuesta_marco = recibir_paquete(socket_memoria);
//   t_respuesta_solicitud_marco* respuesta_operacion_marco = malloc(sizeof(t_respuesta_solicitud_marco));
//   respuesta_operacion_marco = obtener_respuesta_solicitud_marco(paquete_respuesta_marco);
//   printf("Num marco: %d\n", respuesta_operacion_marco->num_marco);

//   // ACCESO PARA OBTENER DATO FISICO
//   t_solicitud_dato_fisico* read_dato = malloc(sizeof(t_solicitud_dato_fisico));
//   obtener_dato_fisico(read_dato, respuesta_operacion_marco->num_marco, num_pagina, tam_pagina, dir_logica);
//   free(read_marco);

//   // RECIBO RESPUESTA DE MEMORIA
//   xlog(COLOR_INFO, "Recibiendo valor desde Memoria ");
//   cod_op = recibir_operacion(socket_memoria);
//   t_paquete* paquete_respuesta_dato = recibir_paquete(socket_memoria);
//   t_respuesta_dato_fisico* respuesta_operacion_dato = malloc(sizeof(t_respuesta_dato_fisico));
//   respuesta_operacion_dato = obtener_respuesta_solicitud_dato_fisico(paquete_respuesta_dato);

//   t_operacion_respuesta_fetch_operands* respuesta_fetch = malloc(sizeof(t_operacion_respuesta_fetch_operands));

//   respuesta_fetch->valor = respuesta_operacion_dato->dato_buscado;

//   return respuesta_fetch;
// }


// void execute_read_write(t_pcb* pcb,
//                         int tam_pagina,
//                         int cant_entradas_por_tabla,
//                         int num_pagina,
//                         uint32_t dir_logica,
//                         void* valor) {
//   log_info(logger, "Leyendo de TLB");
//   bool acierto_tlb = esta_en_tlb(num_pagina);
//   int cod_op = 0;
//   if (acierto_tlb == false) {
//     // SE BUSCA EN MEMORIA LA PAGINA, PARA ELLO SE REALIZAN 3 ACCESOS:
//     // ENVIO EL NUM DE TABLA DE 1er NIVEL JUNTO CON LA ENTRADA A DICHA TABLA
//     // MEMORIA ME DEVUELVE EL NUM DE TABLA DE 2DO NIVEL
//     // LUEGO ENVIO LA ENTRADA DE LA TABLA DE SEGUNDO NIVEL JUNTO CON EL NUM DE TABLA DE 2DO NIVEL
//     // MEMORIA ME DEVUELVE EL NUM DE MARCO
//     // CON ESTO ARMO LA DIRECCION FISICA Y ENVIO A MEMORIA PARA LEER EL DATO (DF= MARCO*TAM MARCO + DESPLAZAMIENTO)

//     log_info(logger, "La pagina no se ecnuentra en la TLB, enviando solicitud a Memoria");

//     // ACCESOS A MEMORIA PARA OBTENER EL MARCO
//     // ACCESO PARA OBTENER TABLA SEGUNDO NIVEL
//     t_solicitud_segunda_tabla* read = malloc(sizeof(t_solicitud_segunda_tabla));
//     obtener_numero_tabla_segundo_nivel(read, pcb, num_pagina, cant_entradas_por_tabla);
//     free(read);


//     // RECIBO RESPUESTA DE MEMORIA
//     xlog(COLOR_INFO, "Recibiendo respuesta de tabla de segundo nivel desde Memoria ");
//     cod_op = recibir_operacion(socket_memoria);
//     t_paquete* paquete_respuesta = recibir_paquete(socket_memoria);
//     t_respuesta_solicitud_segunda_tabla* respuesta_operacion = malloc(sizeof(t_respuesta_solicitud_segunda_tabla));
//     respuesta_operacion = obtener_respuesta_solicitud_tabla_segundo_nivel(paquete_respuesta);
//     printf("Tabla segundo nivel: %d\n", respuesta_operacion->num_tabla_segundo_nivel);

//     // ACCESO PARA OBTENER MARCO
//     t_solicitud_marco* read_marco = malloc(sizeof(t_solicitud_marco));
//     obtener_numero_marco(read_marco, num_pagina, cant_entradas_por_tabla,
//     respuesta_operacion->num_tabla_segundo_nivel);
//     free(read_marco);

//     // RECIBO RESPUESTA DE MEMORIA
//     xlog(COLOR_INFO, "Recibiendo marco nivel desde Memoria ");
//     cod_op = recibir_operacion(socket_memoria);
//     t_paquete* paquete_respuesta_marco = recibir_paquete(socket_memoria);
//     t_respuesta_solicitud_marco* respuesta_operacion_marco = malloc(sizeof(t_respuesta_solicitud_marco));
//     respuesta_operacion_marco = obtener_respuesta_solicitud_marco(paquete_respuesta_marco);
//     printf("Num marco: %d\n", respuesta_operacion_marco->num_marco);

//     if (valor == NULL) { // LECTURA DE DATO
//       // ACCESO PARA OBTENER DATO FISICO
//       t_solicitud_dato_fisico* read_dato = malloc(sizeof(t_solicitud_dato_fisico));
//       obtener_dato_fisico(read_dato, respuesta_operacion_marco->num_marco, num_pagina, tam_pagina, dir_logica);
//       free(read_marco);

//       // RECIBO RESPUESTA DE MEMORIA
//       xlog(COLOR_INFO, "Recibiendo valor desde Memoria ");
//       cod_op = recibir_operacion(socket_memoria);
//       t_paquete* paquete_respuesta_dato = recibir_paquete(socket_memoria);
//       t_respuesta_dato_fisico* respuesta_operacion_dato = malloc(sizeof(t_respuesta_dato_fisico));
//       respuesta_operacion_dato = obtener_respuesta_solicitud_dato_fisico(paquete_respuesta_dato);

//     } else { // ESCRITURA DE DATO
//       t_escritura_dato_fisico* write_dato = malloc(sizeof(t_escritura_dato_fisico));
//       escribir_dato_fisico(write_dato, respuesta_operacion_marco->num_marco, num_pagina, tam_pagina, dir_logica,
//       valor);
//       free(write_dato);

//       // RECIBO RESPUESTA DE MEMORIA
//       xlog(COLOR_INFO, "Recibiendo valor desde Memoria ");
//       cod_op = recibir_operacion(socket_memoria);
//       t_paquete* paquete_respuesta_dato = recibir_paquete(socket_memoria);
//       t_respuesta_escritura_dato_fisico* respuesta_operacion_dato =
//       malloc(sizeof(t_respuesta_escritura_dato_fisico));
//       respuesta_operacion_dato = obtener_respuesta_escritura_dato_fisico(paquete_respuesta_dato);
//     }


//   } else { // Busco el valor en la TLB
//     xlog(COLOR_INFO, "Buscando valor en TLB ");

//     int num_marco = buscar_marco_en_tlb(num_pagina);

//     // ACCESO PARA OBTENER DATO FISICO
//     t_solicitud_dato_fisico* read_dato = malloc(sizeof(t_solicitud_dato_fisico));
//     obtener_dato_fisico(read_dato, num_marco, num_pagina, tam_pagina, dir_logica);
//     free(read_dato);

//     // RECIBO RESPUESTA DE MEMORIA
//     xlog(COLOR_INFO, "Recibiendo respuesta de tabla de segundo nivel desde Memoria ");
//     cod_op = recibir_operacion(socket_memoria);
//     t_paquete* paquete_respuesta = recibir_paquete(socket_memoria);
//     t_respuesta_dato_fisico* respuesta_operacion = malloc(sizeof(t_respuesta_dato_fisico));
//     respuesta_operacion = obtener_respuesta_solicitud_dato_fisico(paquete_respuesta);
//     printf("VALOR BUSCADO: %s\n", respuesta_operacion->dato_buscado);
//   }
// }

// int buscar_marco_en_tlb(int num_pagina) {
//   bool encontrar_entrada(t_entrada_tlb * entrada) {
//     return num_pagina == entrada->pagina;
//   }
//   t_entrada_tlb* entrada_buscada = list_find(tlb, (void*)encontrar_entrada);
//   int marco_buscado = entrada_buscada->marco;
//   free(entrada_buscada);
//   return marco_buscado;
// }

// void obtener_dato_fisico(t_solicitud_dato_fisico* solicitud_dato_fisico,
//                          int num_marco,
//                          int num_pagina,
//                          int tam_pagina,
//                          uint32_t dir_logica) {
//   armar_solicitud_dato_fisico(solicitud_dato_fisico, num_marco, num_pagina, tam_pagina, dir_logica);
//   solicitud_dato_fisico->socket = socket_memoria;
//   /*t_paquete* paquete_con_direccion_a_leer = paquete_create();
//   paquete_add_solicitud_dato_fisico(paquete_con_direccion_a_leer, solicitud_dato_fisico);
//   enviar_operacion_obtener_dato(socket_memoria, paquete_con_direccion_a_leer);
//   paquete_destroy(paquete_con_direccion_a_leer);*/
//   t_paquete* paquete = paquete_create();
//   t_buffer* mensaje = crear_mensaje_obtener_dato_fisico(solicitud_dato_fisico);
//   paquete_cambiar_mensaje(paquete, mensaje), enviar_operacion_obtener_dato(socket_memoria, paquete);
// }


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
          t_buffer* mensaje = crear_mensaje_pcb_actualizado(pcb_deserializado, 0);
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