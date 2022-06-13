#include "cpu.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <libstatic.h> // <-- STATIC LIB
int CONEXION_CPU_INTERRUPT;
int HAY_PCB_PARA_EJECUTAR = 0;
int HAY_INTERRUPCION = 0;
// void* escuchar_dispatch(void* arguments) {

void setear_algoritmo_reemplazo() {
  char* algoritmo = config_get_string_value(config, "REEMPLAZO_TLB");
  if (strcmp(algoritmo, "ALGORITMO_FIFO") == 0) {
    algoritmo_reemplazo = ALGORITMO_FIFO;
  } else
    algoritmo_reemplazo = ALGORITMO_LRU;
}
void* escuchar_dispatch() {
  // struct arg_struct* args = (struct arg_struct*)arguments;
  estado_conexion_kernel = true;

  // memcpy(ip, args->arg1, strlen(args->arg1));
  // memcpy(puerto, args->arg2, strlen(args->arg2));
  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  socket_cpu_dispatch = iniciar_servidor(ip, puerto);

  while (estado_conexion_kernel) {
    int cliente_fd = esperar_cliente(socket_cpu_dispatch);
    /*
        if (cliente_fd != -1) {
          t_paquete* paquete = paquete_create();
          t_buffer* mensaje = crear_mensaje("Conexión aceptada por CPU");

          paquete_cambiar_mensaje(paquete, mensaje), enviar_mensaje(cliente_fd, paquete);
          // paquete_add_mensaje(paquete, mensaje);
        }
    */
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
      case OPERACION_PCB: {
        t_paquete* paquete_con_pcb = malloc(sizeof(t_paquete) + 1);
        paquete_con_pcb = recibir_paquete(socket_cliente);

        t_pcb* pcb_deserializado = paquete_obtener_pcb(paquete_con_pcb);
        // pcb_deserializado->socket = socket_cliente;
        HAY_PCB_PARA_EJECUTAR = 1;
        ciclo_instruccion(pcb_deserializado, socket_cliente);
        imprimir_pcb(pcb_deserializado);
        paquete_destroy(paquete_con_pcb);
        pcb_destroy(pcb_deserializado);
        // descomentar para validar el memcheck
        // terminar_servidor(socket_cpu_dispatch, logger, config);
        // return 0;
      } break;

      /* case OPERACION_RESPUESTA_SEGUNDA_TABLA: {
         xlog(COLOR_INFO, "Se recibe respuesta de tabla de segundo nivel en memoria");
       }

       case OPERACION_RESPUESTA_MARCO: {
         xlog(COLOR_INFO, "Se recibe respuesta del proceso de busqueda en memoria");
       }

       case OPERACION_BUSQUEDA_EN_MEMORIA_OK: {
         xlog(COLOR_INFO, "Se recibe respuesta del proceso de busqueda en memoria");
         break;
       }*/
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

        log_destroy(logger), close(socket_cpu_dispatch);
        // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
        estado_conexion_kernel = false;
        estado_conexion_con_cliente = false;
      } break;
      case -1: {
        log_info(logger, "el cliente se desconecto");
        // cliente_estado = CLIENTE_EXIT;
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

void ciclo_instruccion(t_pcb* pcb, int socket_cliente) {
  log_info(logger, "Iniciando ciclo de instruccion");
  log_info(logger, "leyendo instrucciones");

  while (HAY_PCB_PARA_EJECUTAR && pcb->program_counter < list_size(pcb->instrucciones)) {
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    instruccion = fetch(pcb);
    pcb->program_counter++;
    decode(instruccion, pcb, socket_cliente);
    // free(instruccion);
    check_interrupt(pcb, socket_cliente);
  }
}

t_instruccion* fetch(t_pcb* pcb) {
  return list_get(pcb->instrucciones, pcb->program_counter);
}

void decode(t_instruccion* instruccion, t_pcb* pcb, int socket_cliente) {
  if (strcmp(instruccion->identificador, "NO_OP") == 0) {
    log_info(logger, "Ejecutando NO_OP...");
    execute_no_op();
  }

  else if (strcmp(instruccion->identificador, "I/O") == 0) {
    log_info(logger, "Ejecutando IO...");

    execute_io(pcb, instruccion, socket_cliente);

    // paquete_destroy(paquete_con_pcb);
  }

  else if (strcmp(instruccion->identificador, "READ") ==
           0) { /// DE MEMORIA ANTICIPADAMENTE SE TIENE QUE TRAER EL TAM DE PAGINA PARA HACER LA TRADUCCION DESDE LA TLB
    log_info(logger, "Ejecutando READ...");
    int tam_pagina = 64; // TODO: ESTE NUMERO LO TIENE QUE TRAER DE MEMORIA. USAR SOLO PARA PRUEBAS
    int cant_entradas_por_tabla = 10;
    int num_pagina = (float)atoi(instruccion->params) / tam_pagina;
    uint32_t dir_logica = atoi(instruccion->params);

    // int tabla_primer_nivel = *pcb->tabla_primer_nivel;

    pcb->tabla_primer_nivel = 1;

    execute_read_write(pcb, tam_pagina, cant_entradas_por_tabla, num_pagina, dir_logica, 0, 1);
  }

  else if (strcmp(instruccion->identificador, "WRITE") == 0) {
    log_info(logger, "Ejecutando WRITE...");
    int tam_pagina = 64; // TODO: ESTE NUMERO LO TIENE QUE TRAER DE MEMORIA. USAR SOLO PARA PRUEBAS
    int cant_entradas_por_tabla = 10;
    char** params = string_split(instruccion->params, " ");
    uint32_t dir_logica = atoi(params[0]);
    uint32_t valor = atoi(params[1]);
    int num_pagina = (float)dir_logica / tam_pagina;
    // uint32_t dir_logica = atoi(instruccion->params);
    execute_read_write(pcb, tam_pagina, cant_entradas_por_tabla, num_pagina, dir_logica, valor, 2);
    free(params);
  }

  else if (strcmp(instruccion->identificador, "COPY") == 0) {
    xlog(COLOR_CONEXION, "Ejecutando COPY");
    int tam_pagina = 64; // TODO: ESTE NUMERO LO TIENE QUE TRAER DE MEMORIA. USAR SOLO PARA PRUEBAS
    int cant_entradas_por_tabla = 10;
    char** params = string_split(instruccion->params, " ");
    uint32_t dir_logica_destino = atoi(params[0]);
    uint32_t dir_logica_origen = atoi(params[1]);
    int num_pagina = (float)dir_logica_origen / tam_pagina;
    t_operacion_respuesta_fetch_operands* respuesta_fetch =
      fetch_operands(pcb, tam_pagina, cant_entradas_por_tabla, num_pagina, dir_logica_origen, 3);
    execute_read_write(
      pcb, tam_pagina, cant_entradas_por_tabla, num_pagina, dir_logica_origen, respuesta_fetch->valor, 2);

  }


  else if (strcmp(instruccion->identificador, "EXIT") == 0) {
    xlog(COLOR_CONEXION, "Ejecutando EXIT");
    execute_exit(pcb, socket_cliente);
  }
}
/*
void execute_no_op() {
  int retardo = config_get_int_value(config, "RETARDO_NOOP");
  xlog(COLOR_INFO, "Retardo de NO_OP en milisegundos: %d", retardo);
  usleep(retardo * 1000);
}

void execute_io(t_pcb* pcb, t_instruccion* instruccion, int socket_cliente) {
  int tiempo_bloqueado = instruccion_obtener_parametro(instruccion, 0);
  pcb->tiempo_de_bloqueado = tiempo_bloqueado;

  t_paquete* paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  xlog(COLOR_INFO, "Se actualizó el tiempo de bloqueo de un proceso (pid=%d, tiempo=%d)", pcb->pid, tiempo_bloqueado);
  enviar_pcb_con_operacion_io(socket_cliente, paquete);
  HAY_PCB_PARA_EJECUTAR = 0;
}
void execute_exit(t_pcb* pcb, int socket_cliente) {
  // pcb->program_counter++;
  t_paquete* paquete = paquete_create();
  t_buffer* mensaje = crear_mensaje_pcb_actualizado(pcb, 0);
  paquete_cambiar_mensaje(paquete, mensaje);
  enviar_pcb_actualizado(socket_cliente, paquete);
}*/

void execute_no_op() {
  int retardo = config_get_int_value(config, "RETARDO_NOOP");
  xlog(COLOR_INFO, "Retardo de NO_OP en milisegundos: %d", retardo);
  usleep(retardo * 1000);
}

void execute_io(t_pcb* pcb, t_instruccion* instruccion, int socket_cliente) {
  int tiempo_bloqueado = instruccion_obtener_parametro(instruccion, 0);
  pcb->tiempo_de_bloqueado = tiempo_bloqueado;

  t_paquete* paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  xlog(COLOR_INFO, "Se actualizó el tiempo de bloqueo de un proceso (pid=%d, tiempo=%d)", pcb->pid, tiempo_bloqueado);
  enviar_pcb_con_operacion_io(socket_cliente, paquete);
  HAY_PCB_PARA_EJECUTAR = 0;
}
void execute_exit(t_pcb* pcb, int socket_cliente) {
  // pcb->program_counter++;
  t_paquete* paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  enviar_pcb_con_operacion_exit(socket_cliente, paquete);
  HAY_PCB_PARA_EJECUTAR = 0;
  /*
  t_buffer* mensaje = crear_mensaje_pcb_actualizado(pcb, NULL);
  paquete_cambiar_mensaje(paquete, mensaje);
  enviar_pcb_actualizado(socket_cliente, paquete);
  */
}

t_operacion_respuesta_fetch_operands* fetch_operands(t_pcb* pcb,
                                                     int tam_pagina,
                                                     int cant_entradas_por_tabla,
                                                     int num_pagina,
                                                     uint32_t dir_logica,
                                                     int operacion) {
  log_info(logger, "La pagina no se ecnuentra en la TLB, enviando solicitud a Memoria");
  int cod_op = 0;

  // ACCESOS A MEMORIA PARA OBTENER EL MARCO
  // ACCESO PARA OBTENER TABLA SEGUNDO NIVEL
  t_solicitud_segunda_tabla* read = malloc(sizeof(t_solicitud_segunda_tabla));
  pcb->tabla_primer_nivel = 1;
  obtener_numero_tabla_segundo_nivel(read, pcb, num_pagina, cant_entradas_por_tabla);
  free(read);


  // RECIBO RESPUESTA DE MEMORIA
  xlog(COLOR_INFO, "Recibiendo respuesta de tabla de segundo nivel desde Memoria ");
  cod_op = recibir_operacion(socket_memoria);
  t_paquete* paquete_respuesta = recibir_paquete(socket_memoria);
  t_respuesta_solicitud_segunda_tabla* respuesta_operacion = malloc(sizeof(t_respuesta_solicitud_segunda_tabla));
  respuesta_operacion = obtener_respuesta_solicitud_tabla_segundo_nivel(paquete_respuesta);
  printf("Tabla segundo nivel: %d\n", respuesta_operacion->num_tabla_segundo_nivel);

  // ACCESO PARA OBTENER MARCO
  t_solicitud_marco* read_marco = malloc(sizeof(t_solicitud_marco));
  obtener_numero_marco(
    read_marco, num_pagina, cant_entradas_por_tabla, respuesta_operacion->num_tabla_segundo_nivel, operacion);
  free(read_marco);

  // RECIBO RESPUESTA DE MEMORIA
  xlog(COLOR_INFO, "Recibiendo marco nivel desde Memoria ");
  cod_op = recibir_operacion(socket_memoria);
  t_paquete* paquete_respuesta_marco = recibir_paquete(socket_memoria);
  t_respuesta_solicitud_marco* respuesta_operacion_marco = malloc(sizeof(t_respuesta_solicitud_marco));
  respuesta_operacion_marco = obtener_respuesta_solicitud_marco(paquete_respuesta_marco);
  printf("Num marco: %d\n", respuesta_operacion_marco->num_marco);

  // ACCESO PARA OBTENER DATO FISICO
  t_solicitud_dato_fisico* read_dato = malloc(sizeof(t_solicitud_dato_fisico));
  obtener_dato_fisico(read_dato, respuesta_operacion_marco->num_marco, num_pagina, tam_pagina, dir_logica);
  free(read_marco);

  // RECIBO RESPUESTA DE MEMORIA
  xlog(COLOR_INFO, "Recibiendo valor desde Memoria ");
  cod_op = recibir_operacion(socket_memoria);
  t_paquete* paquete_respuesta_dato = recibir_paquete(socket_memoria);
  t_respuesta_dato_fisico* respuesta_operacion_dato = malloc(sizeof(t_respuesta_dato_fisico));
  respuesta_operacion_dato = obtener_respuesta_solicitud_dato_fisico(paquete_respuesta_dato);

  t_operacion_respuesta_fetch_operands* respuesta_fetch = malloc(sizeof(t_operacion_respuesta_fetch_operands));

  respuesta_fetch->valor = respuesta_operacion_dato->dato_buscado;

  return respuesta_fetch;
}


void execute_read_write(t_pcb* pcb,
                        int tam_pagina,
                        int cant_entradas_por_tabla,
                        int num_pagina,
                        uint32_t dir_logica,
                        uint32_t valor,
                        int operacion) {
  log_info(logger, "Leyendo de TLB");
  bool acierto_tlb = esta_en_tlb(num_pagina);
  int cod_op = 0;
  if (acierto_tlb == false) {
    // SE BUSCA EN MEMORIA LA PAGINA, PARA ELLO SE REALIZAN 3 ACCESOS:
    // ENVIO EL NUM DE TABLA DE 1er NIVEL JUNTO CON LA ENTRADA A DICHA TABLA
    // MEMORIA ME DEVUELVE EL NUM DE TABLA DE 2DO NIVEL
    // LUEGO ENVIO LA ENTRADA DE LA TABLA DE SEGUNDO NIVEL JUNTO CON EL NUM DE TABLA DE 2DO NIVEL
    // MEMORIA ME DEVUELVE EL NUM DE MARCO
    // CON ESTO ARMO LA DIRECCION FISICA Y ENVIO A MEMORIA PARA LEER EL DATO (DF= MARCO*TAM MARCO + DESPLAZAMIENTO)

    log_info(logger, "La pagina no se ecnuentra en la TLB, enviando solicitud a Memoria");

    // ACCESO PARA OBTENER TABLA SEGUNDO NIVEL
    pcb->tabla_primer_nivel = 1;
    t_solicitud_segunda_tabla* read = malloc(sizeof(t_solicitud_segunda_tabla));
    obtener_numero_tabla_segundo_nivel(read, pcb, num_pagina, cant_entradas_por_tabla);
    free(read);


    // RECIBO RESPUESTA DE MEMORIA
    xlog(COLOR_INFO, "Recibiendo respuesta de tabla de segundo nivel desde Memoria ");
    cod_op = recibir_operacion(socket_memoria);
    t_paquete* paquete_respuesta = recibir_paquete(socket_memoria);
    t_respuesta_solicitud_segunda_tabla* respuesta_operacion = malloc(sizeof(t_respuesta_solicitud_segunda_tabla));
    respuesta_operacion = obtener_respuesta_solicitud_tabla_segundo_nivel(paquete_respuesta);
    printf("Tabla segundo nivel: %d\n", respuesta_operacion->num_tabla_segundo_nivel);

    // ACCESO PARA OBTENER MARCO
    t_solicitud_marco* read_marco = malloc(sizeof(t_solicitud_marco));
    obtener_numero_marco(
      read_marco, num_pagina, cant_entradas_por_tabla, respuesta_operacion->num_tabla_segundo_nivel, operacion);
    free(read_marco);

    // RECIBO RESPUESTA DE MEMORIA
    xlog(COLOR_INFO, "Recibiendo marco nivel desde Memoria ");
    cod_op = recibir_operacion(socket_memoria);
    t_paquete* paquete_respuesta_marco = recibir_paquete(socket_memoria);
    t_respuesta_solicitud_marco* respuesta_operacion_marco = malloc(sizeof(t_respuesta_solicitud_marco));
    respuesta_operacion_marco = obtener_respuesta_solicitud_marco(paquete_respuesta_marco);
    printf("Num marco: %d\n", respuesta_operacion_marco->num_marco);

    if (operacion == 1) { // LECTURA DE DATO
      // ACCESO PARA OBTENER DATO FISICO
      t_solicitud_dato_fisico* read_dato = malloc(sizeof(t_solicitud_dato_fisico));
      obtener_dato_fisico(read_dato, respuesta_operacion_marco->num_marco, num_pagina, tam_pagina, dir_logica);
      free(read_dato);

      // RECIBO RESPUESTA DE MEMORIA
      xlog(COLOR_INFO, "Recibiendo valor desde Memoria ");
      cod_op = recibir_operacion(socket_memoria);
      t_paquete* paquete_respuesta_dato = recibir_paquete(socket_memoria);
      t_respuesta_dato_fisico* respuesta_operacion_dato = malloc(sizeof(t_respuesta_dato_fisico));
      respuesta_operacion_dato = obtener_respuesta_solicitud_dato_fisico(paquete_respuesta_dato);
      printf("DATO: %d\n", respuesta_operacion_dato->dato_buscado);

    } else { // ESCRITURA DE DATO
      t_escritura_dato_fisico* write_dato = malloc(sizeof(t_escritura_dato_fisico));
      escribir_dato_fisico(
        write_dato, respuesta_operacion_marco->num_marco, num_pagina, tam_pagina, dir_logica, write_dato->valor);
      free(write_dato);

      // RECIBO RESPUESTA DE MEMORIA
      xlog(COLOR_INFO, "Recibiendo valor desde Memoria ");
      cod_op = recibir_operacion(socket_memoria);
      t_paquete* paquete_respuesta_dato = recibir_paquete(socket_memoria);
      t_respuesta_escritura_dato_fisico* respuesta_operacion_dato = malloc(sizeof(t_respuesta_escritura_dato_fisico));
      respuesta_operacion_dato = obtener_respuesta_escritura_dato_fisico(paquete_respuesta_dato);
      printf("RESULTADO: %d\n", respuesta_operacion_dato->resultado);
    }


  } else { // Busco el valor en la TLB
    xlog(COLOR_INFO, "Buscando valor en TLB ");

    int num_marco = buscar_marco_en_tlb(num_pagina);

    // ACCESO PARA OBTENER DATO FISICO
    t_solicitud_dato_fisico* read_dato = malloc(sizeof(t_solicitud_dato_fisico));
    obtener_dato_fisico(read_dato, num_marco, num_pagina, tam_pagina, dir_logica);
    free(read_dato);

    // RECIBO RESPUESTA DE MEMORIA
    xlog(COLOR_INFO, "Recibiendo respuesta de tabla de segundo nivel desde Memoria ");
    cod_op = recibir_operacion(socket_memoria);
    t_paquete* paquete_respuesta = recibir_paquete(socket_memoria);
    t_respuesta_dato_fisico* respuesta_operacion = malloc(sizeof(t_respuesta_dato_fisico));
    respuesta_operacion = obtener_respuesta_solicitud_dato_fisico(paquete_respuesta);
    printf("VALOR BUSCADO: %d\n", respuesta_operacion->dato_buscado);
  }
}

int buscar_marco_en_tlb(int num_pagina) {
  bool encontrar_entrada(t_entrada_tlb * entrada) {
    return num_pagina == entrada->pagina;
  }
  t_entrada_tlb* entrada_buscada = list_find(tlb, (void*)encontrar_entrada);
  int marco_buscado = entrada_buscada->marco;
  free(entrada_buscada);
  return marco_buscado;
}

void obtener_dato_fisico(t_solicitud_dato_fisico* solicitud_dato_fisico,
                         int num_marco,
                         int num_pagina,
                         int tam_pagina,
                         uint32_t dir_logica) {
  armar_solicitud_dato_fisico(solicitud_dato_fisico, num_marco, num_pagina, tam_pagina, dir_logica);
  solicitud_dato_fisico->socket = socket_memoria;
  /*t_paquete* paquete_con_direccion_a_leer = paquete_create();
  paquete_add_solicitud_dato_fisico(paquete_con_direccion_a_leer, solicitud_dato_fisico);
  enviar_operacion_obtener_dato(socket_memoria, paquete_con_direccion_a_leer);
  paquete_destroy(paquete_con_direccion_a_leer);*/
  t_paquete* paquete = paquete_create();
  t_buffer* mensaje = crear_mensaje_obtener_dato_fisico(solicitud_dato_fisico);
  paquete_cambiar_mensaje(paquete, mensaje), enviar_operacion_obtener_dato(socket_memoria, paquete);
}

void escribir_dato_fisico(t_escritura_dato_fisico* escritura_dato_fisico,
                          int num_marco,
                          int num_pagina,
                          int tam_pagina,
                          uint32_t dir_logica,
                          uint32_t valor) {
  armar_escritura_dato_fisico(escritura_dato_fisico, num_marco, num_pagina, tam_pagina, dir_logica, valor);
  escritura_dato_fisico->socket = socket_memoria;
  /*t_paquete* paquete_con_direccion_a_leer = paquete_create();
  paquete_add_solicitud_dato_fisico(paquete_con_direccion_a_leer, solicitud_dato_fisico);
  enviar_operacion_obtener_dato(socket_memoria, paquete_con_direccion_a_leer);
  paquete_destroy(paquete_con_direccion_a_leer);*/
  t_paquete* paquete = paquete_create();
  t_buffer* mensaje = crear_mensaje_escritura_dato_fisico(escritura_dato_fisico);
  paquete_cambiar_mensaje(paquete, mensaje), enviar_operacion_escribir_dato(socket_memoria, paquete);
}

void obtener_numero_marco(t_solicitud_marco* solicitud_marco,
                          int num_pagina,
                          int cant_entradas_por_tabla,
                          int numero_tabla_segundo_nivel,
                          int operacion) {
  armar_solicitud_marco(solicitud_marco, num_pagina, cant_entradas_por_tabla, numero_tabla_segundo_nivel, operacion);
  solicitud_marco->socket = socket_memoria;
  /*t_paquete* paquete_con_direccion_a_leer = paquete_create();
  paquete_add_solicitud_marco(paquete_con_direccion_a_leer, solicitud_marco);
  enviar_operacion_obtener_marco(socket_memoria, paquete_con_direccion_a_leer);
  paquete_destroy(paquete_con_direccion_a_leer);*/
  t_paquete* paquete = paquete_create();
  t_buffer* mensaje = crear_mensaje_obtener_marco(solicitud_marco);
  paquete_cambiar_mensaje(paquete, mensaje), enviar_operacion_obtener_marco(socket_memoria, paquete);
}

void obtener_numero_tabla_segundo_nivel(t_solicitud_segunda_tabla* read,
                                        t_pcb* pcb,
                                        int num_pagina,
                                        int cant_entradas_por_tabla) {
  armar_solicitud_tabla_segundo_nivel(read, pcb->tabla_primer_nivel, num_pagina, cant_entradas_por_tabla);
  read->socket = socket_memoria;
  // t_paquete* paquete_con_direccion_a_leer = paquete_create();
  // paquete_add_solicitud_tabla_segundo_nivel(paquete_con_direccion_a_leer, read);
  /*enviar_operacion_obtener_segunda_tabla(socket_memoria, paquete_con_direccion_a_leer);
  paquete_destroy(paquete_con_direccion_a_leer);*/

  t_paquete* paquete = paquete_create();
  t_buffer* mensaje = crear_mensaje_obtener_segunda_tabla(read);
  paquete_cambiar_mensaje(paquete, mensaje), enviar_operacion_obtener_segunda_tabla(socket_memoria, paquete);
}


void armar_solicitud_tabla_segundo_nivel(t_solicitud_segunda_tabla* solicitud_tabla_segundo_nivel,
                                         int num_tabla_primer_nivel,
                                         int num_pagina,
                                         int cant_entradas_por_tabla) {
  solicitud_tabla_segundo_nivel->num_tabla_primer_nivel = num_tabla_primer_nivel;
  solicitud_tabla_segundo_nivel->entrada_primer_nivel = (float)num_pagina / (float)cant_entradas_por_tabla;
  xlog(COLOR_INFO, "Entrada Tabla 1er nivel: %d", solicitud_tabla_segundo_nivel->entrada_primer_nivel);
}

void armar_solicitud_marco(t_solicitud_marco* solicitud_marco,
                           int num_pagina,
                           int cant_entradas_por_tabla,
                           int numero_tabla_segundo_nivel,
                           int operacion) {
  solicitud_marco->num_tabla_segundo_nivel = numero_tabla_segundo_nivel;
  solicitud_marco->entrada_segundo_nivel = num_pagina % cant_entradas_por_tabla;
  solicitud_marco->operacion = operacion;
  xlog(COLOR_INFO, "SOLICITUD MARCO: TABLA SEGUNDO NIVEL: %d", solicitud_marco->entrada_segundo_nivel);
}

void armar_operacion_read(t_operacion_read* read, t_instruccion* instruccion) {
  read->direccion_logica = atoi(instruccion->params);
}

void armar_solicitud_dato_fisico(t_solicitud_dato_fisico* solicitud_dato_fisico,
                                 int num_marco,
                                 int num_pagina,
                                 int tam_pagina,
                                 uint32_t dir_logica) {
  int desplazamiento = dir_logica - (num_pagina * tam_pagina);
  solicitud_dato_fisico->dir_fisica = num_marco * tam_pagina + desplazamiento;
}

void armar_escritura_dato_fisico(t_escritura_dato_fisico* escritura_dato_fisico,
                                 int num_marco,
                                 int num_pagina,
                                 int tam_pagina,
                                 uint32_t dir_logica,
                                 uint32_t valor) {
  int desplazamiento = dir_logica - (num_pagina * tam_pagina);
  escritura_dato_fisico->dir_fisica = num_marco * tam_pagina + desplazamiento;
  escritura_dato_fisico->valor = valor;
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

bool esta_en_tlb(int num_pagina) {
  bool es_la_pagina(t_entrada_tlb * contenido_tlb) {
    return contenido_tlb->pagina == num_pagina;
  }
  return list_any_satisfy(tlb, (void*)es_la_pagina);
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
    int socket_cliente = esperar_cliente(CONEXION_CPU_INTERRUPT);
    estado_conexion_con_cliente = CONEXION_ESCUCHANDO;

    while (estado_conexion_con_cliente) {
      int codigo_operacion = recibir_operacion(socket_cliente);

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
          HAY_INTERRUPCION = 1;
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

void ejecutar_reemplazo() {
  switch (algoritmo_reemplazo) {
    case ALGORITMO_FIFO: {
      reemplazo_fifo();

      break;
    }

    case ALGORITMO_LRU: {
      reemplazo_lru();
      break;
    }
  }
}

void reemplazo_fifo(t_entrada_tlb* entrada_reemplazo) {
  while (puntero_reemplazo < list_size(tlb) - 1) {
    t_entrada_tlb* entrada_actual = list_get(tlb, puntero_reemplazo);
    list_replace(tlb, puntero_reemplazo, entrada_actual);
    puntero_reemplazo++;
  }

  if (puntero_reemplazo == list_size(tlb) - 1) {
    puntero_reemplazo = 0;
  }
}

void reemplazo_lru() {
  t_entrada_tlb* entrada_buscada = malloc(sizeof(t_entrada_tlb));
  uint64_t aux_timestamp = UINT64_MAX;

  void search_oldest(void* elemento) {
    t_entrada_tlb* entrada = (t_entrada_tlb*)elemento;

    if (entrada->timestamp < aux_timestamp) {
      aux_timestamp = entrada->timestamp;
      entrada_buscada = entrada;
    }
  }

  list_iterate(tlb, search_oldest);
}

int instruccion_obtener_parametro(t_instruccion* instruccion, int numero_parametro) {
  char** parametros = string_split(instruccion->params, " ");
  int valor = atoi(parametros[numero_parametro]);

  string_iterate_lines(parametros, (void*)free);

  return valor;
}

void check_interrupt(t_pcb* pcb, int socket_cliente) {
  if (HAY_PCB_PARA_EJECUTAR) {
    if (HAY_INTERRUPCION) {
      t_paquete* paquete = paquete_create();
      paquete_add_pcb(paquete, pcb);
      enviar_pcb_desalojado(socket_cliente, paquete);
      xlog(COLOR_TAREA, "Se ha desalojado un PCB de CPU (pcb=%d)", pcb->pid);
      HAY_PCB_PARA_EJECUTAR = 0;
      HAY_INTERRUPCION = 0;
    }
  } else {
    HAY_INTERRUPCION = 0; // Para el caso en el que no haya pcb pero se haya mandado una interrupcion
  }
}