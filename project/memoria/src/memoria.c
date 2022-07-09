#include "memoria.h"
#include "libstatic.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/collections/dictionary.h>
#include <commons/memory.h>
#include <commons/string.h>

// TODO: validar si remover, ya no se está utilizando
void* reservar_memoria_inicial(int size_memoria_total) {
  void* memoria_total = malloc(size_memoria_total);

  memset(memoria_total, 0, size_memoria_total);

  return memoria_total;
}

// TODO: validar
void* escuchar_conexiones() {
  estado_conexion_memoria = true;
  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA");
  socket_memoria = iniciar_servidor(ip, puerto);

  while (estado_conexion_memoria) {
    int cliente_fd = esperar_cliente(socket_memoria);
    /*
        if (cliente_fd != -1) {
          t_paquete* paquete = paquete_create();
          t_buffer* mensaje = crear_mensaje("Conexión aceptada por MEMORIA");

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

// TODO: validar
void* manejar_nueva_conexion(void* args) {
  int socket_cliente = *(int*)args;
  estado_conexion_con_cliente = true;
  while (estado_conexion_con_cliente) {
    int codigo_operacion = recibir_operacion(socket_cliente);

    switch (codigo_operacion) {
      case MENSAJE_HANDSHAKE: {
        xlog(COLOR_CONEXION, "Handshake cpu - Se recibio solicitud handshake");
        t_paquete* paquete = recibir_paquete(socket_cliente);
        paquete_destroy(paquete);

        uint32_t entradas_por_tabla = config_get_int_value(config, "ENTRADAS_POR_TABLA");
        uint32_t tam_pagina = tamanio_marco; // config_get_int_value(config, "TAM_PAGINA");
        t_mensaje_handshake_cpu_memoria* mensaje_handshake = mensaje_handshake_create(entradas_por_tabla, tam_pagina);

        t_paquete* paquete_con_respuesta = paquete_create();
        paquete_add_mensaje_handshake(paquete_con_respuesta, mensaje_handshake);
        enviar_mensaje_handshake(socket_cliente, paquete_con_respuesta);
        paquete_destroy(paquete_con_respuesta);

        break;
      }
      case OPERACION_MENSAJE: {
        recibir_mensaje(socket_cliente);

        // t_paquete* paquete = recibir_paquete(cliente_fd);
        // t_mensaje_handshake_cpu_memoria* mensaje = paquete_obtener_mensaje_handshake(paquete);

      } break;

      // TODO: validar porque quedó comentado
      case READ: {
        /*log_info(logger, "Comenzando operacion READ...");
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_solicitud_segunda_tabla* read = paquete_obtener_solicitud_tabla_segundo_nivel(paquete);

        log_info(logger, "Paquete recibido...");

        // PROCESO EL VALOR ENVIADO POR CPU, POR AHORA HARDCODEO UN VALOR PARA PROBAR LA CONEXION

        t_respuesta_operacion_read* respuesta_read = malloc(sizeof(t_respuesta_operacion_read));
        respuesta_read->valor_buscado = 3;
        t_paquete* paquete_con_respuesta = paquete_create();
        paquete_add_respuesta_operacion_read(paquete_con_respuesta, respuesta_read);
        enviar_operacion_read(socket_cliente, paquete_con_respuesta);
        // DESCOMENTAR PARA RESOLVER SEG FAULT
        paquete_destroy(paquete_con_respuesta);

        free(respuesta_read);*/
        break;
      }

      // TODO: validar porque quedó comentado
      /*
      case OPERACION_INICIALIZAR_ESTRUCTURAS: {
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_pcb* pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        // TODO: resolver cuando se avance el módulo..

        xlog(COLOR_CONEXION, "Se recibió solicitud de Kernel para inicializar estructuras de un proceso");

        pcb->tabla_primer_nivel = 1;
        t_paquete* paquete_con_pcb_actualizado = paquete_create();
        paquete_add_pcb(paquete_con_pcb_actualizado, pcb);


        // TODO: deberia agregar al pcb el valor de la tabla de paginas
        confirmar_estructuras_en_memoria(socket_cliente, paquete_con_pcb_actualizado);
        paquete_destroy(paquete_con_pcb_actualizado);
      } break;
      */
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

        log_destroy(logger), close(socket_memoria);
        // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
        estado_conexion_memoria = false;
        estado_conexion_con_cliente = false;

        break;
      }
      case OPERACION_OBTENER_SEGUNDA_TABLA: {
        xlog(COLOR_CONEXION, "Obteniendo numero de tabla de segundo nivel");
        // codigo_operacion = recibir_operacion(socket_memoria);
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_solicitud_segunda_tabla* solicitud_numero_tp_segundo_nivel = malloc(sizeof(t_solicitud_segunda_tabla));

        solicitud_numero_tp_segundo_nivel = obtener_solicitud_tabla_segundo_nivel(paquete);

        int numero_TP_segundo_nivel =
          obtener_numero_TP_segundo_nivel(solicitud_numero_tp_segundo_nivel->num_tabla_primer_nivel, solicitud_numero_tp_segundo_nivel->entrada_primer_nivel);
        // BORRAR GASTON - DESCOMENTAR LINEAS DE ARRIBA Y BORRAR LINEA DE ABAJO
        // int numero_TP_segundo_nivel = 5;

        xlog(COLOR_INFO, "SEGUNDA TABLA: %d", numero_TP_segundo_nivel);

        // TODO: validar si no hay una función que agregue el contenido más fácil ó crear una abstracción
        t_paquete* paquete_respuesta = paquete_create();
        t_respuesta_solicitud_segunda_tabla* resp = malloc(sizeof(t_respuesta_solicitud_segunda_tabla));
        resp->socket = socket_cliente;
        resp->num_tabla_segundo_nivel = numero_TP_segundo_nivel;

        t_buffer* mensaje = crear_mensaje_respuesta_segunda_tabla(resp);
        paquete_cambiar_mensaje(paquete_respuesta, mensaje), enviar_operacion_respuesta_segunda_tabla(socket_cliente, paquete_respuesta);

        free(paquete_respuesta);
        paquete_destroy(paquete);

        break;
      }
      case OPERACION_OBTENER_MARCO: {
        xlog(COLOR_CONEXION, "Obteniendo numero de marco");
        // codigo_operacion = recibir_operacion(socket_memoria);
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_solicitud_marco* solicitud_numero_marco = malloc(sizeof(t_solicitud_marco));

        solicitud_numero_marco = obtener_solicitud_marco(paquete);


        // TODO: evaluar como responder si la TP_segundo_nivel no tiene la entrada, responder con un error de operacion?
        int num_marco = obtener_marco(solicitud_numero_marco->num_tabla_segundo_nivel, solicitud_numero_marco->entrada_segundo_nivel);
        // BORRAR GASTON - DESCOMENTAR LINEAS DE ARRIBA Y BORRAR LINEA DE ABAJO
        // int num_marco = 4;
        xlog(COLOR_INFO, "NUMERO MARCO: %d", num_marco);


        // TODO: validar si no hay una función que agregue el contenido más fácil ó crear una abstracción
        t_paquete* paquete_respuesta = paquete_create();
        t_respuesta_solicitud_marco* resp = malloc(sizeof(t_respuesta_solicitud_marco));
        resp->num_marco = num_marco;
        t_buffer* mensaje = crear_mensaje_respuesta_marco(resp);
        paquete_cambiar_mensaje(paquete_respuesta, mensaje), enviar_operacion_obtener_marco(socket_cliente, paquete_respuesta);

        free(paquete_respuesta);
        free(paquete);
        break;
      }
      case OPERACION_OBTENER_DATO: {
        xlog(COLOR_CONEXION, "Buscando dato pedido en MEMORIA PRINCIPAL");

        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_solicitud_dato_fisico* req = malloc(sizeof(t_solicitud_dato_fisico));
        req = obtener_solicitud_dato(paquete);

        uint32_t direccion_fisica = req->dir_fisica;
        free(req);

        uint32_t dato_buscado = buscar_dato_en_memoria(direccion_fisica);

        t_paquete* paquete_respuesta = paquete_create();
        t_respuesta_dato_fisico* resp = malloc(sizeof(t_respuesta_dato_fisico));

        memcpy(&(resp->dato_buscado), &dato_buscado, sizeof(uint32_t));

        t_buffer* mensaje = crear_mensaje_respuesta_dato_fisico(resp);
        paquete_cambiar_mensaje(paquete_respuesta, mensaje), enviar_operacion_obtener_dato(socket_cliente, paquete_respuesta);

        free(resp);
        free(paquete_respuesta);

        break;
      }

      case OPERACION_ESCRIBIR_DATO: {
        xlog(COLOR_CONEXION, "Escribiendo dato en MEMORIA PRINCIPAL");

        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_escritura_dato_fisico* req = malloc(sizeof(t_escritura_dato_fisico));
        req = obtener_solicitud_escritura_dato(paquete);

        uint32_t direccion_fisica = req->dir_fisica;
        uint32_t valor = req->valor;
        free(req);

        uint32_t resultado_escritura = escribir_dato(direccion_fisica, valor);

        t_paquete* paquete_respuesta = paquete_create();
        t_respuesta_escritura_dato_fisico* resp = malloc(sizeof(t_respuesta_escritura_dato_fisico));
        resp->resultado = resultado_escritura;
        t_buffer* mensaje = crear_mensaje_respuesta_escritura_dato_fisico(resp);
        paquete_cambiar_mensaje(paquete_respuesta, mensaje), enviar_operacion_escribir_dato(socket_cliente, paquete_respuesta);

        free(resp);
        free(paquete_respuesta);

        break;
      }
      case OPERACION_PROCESO_SUSPENDIDO: {
        t_paquete* paquete = recibir_paquete(socket_cliente);
        // TODO: resolver cuando se avance el módulo..
        // TODO: Escribir en swap paginas con bit M en 1
        xlog(COLOR_CONEXION, "Se recibió solicitud de Kernel para suspender proceso");
        confirmar_suspension_de_proceso(socket_cliente, paquete);
        paquete_destroy(paquete);
      } break;
      case OPERACION_INICIALIZAR_ESTRUCTURAS: {
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_pcb* pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        // TODO: resolver cuando se avance el módulo..

        xlog(COLOR_CONEXION, "Se recibió solicitud de Kernel para inicializar estructuras de un proceso");


        // TODO: DEVOLVER LA TABLA DE IMER NIVEL POSTA ASIGNADA AL PROCESO
        pcb->tabla_primer_nivel = 1;
        t_paquete* paquete_con_pcb_actualizado = paquete_create();
        paquete_add_pcb(paquete_con_pcb_actualizado, pcb);


        // TODO: deberia agregar al pcb el valor de la tabla de paginas
        confirmar_estructuras_en_memoria(socket_cliente, paquete_con_pcb_actualizado);
        paquete_destroy(paquete_con_pcb_actualizado);
      } break;
      case OPERACION_PROCESO_FINALIZADO: {
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_pcb* pcb = paquete_obtener_pcb(paquete);
        // TODO: resolver cuando se avance el módulo de memoria
        xlog(COLOR_CONEXION, "Memoria/Swap recibió solicitud de Kernel para liberar las estructuras de un proceso");
        // TODO: Agregar retardo de swap?
        liberar_estructuras_en_swap(pcb->pid);

        paquete_destroy(paquete);
      } break;
      case -1: {
        log_info(logger, "el cliente se desconecto");
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

bool tiene_marco_asignado_entrada_TP(t_entrada_tabla_segundo_nivel* entrada) {
  return entrada->num_marco != -1;
}

// TODO: validar
int obtener_marco(int numero_tabla_paginas_segundo_nivel, int numero_entrada_TP_segundo_nivel) {
  int marco = 0;
  xlog(COLOR_TAREA, "Buscando un marco disponible... (TP_2do_nivel=%d, numero_entrada=%d)", numero_tabla_paginas_segundo_nivel, numero_entrada_TP_segundo_nivel);

  // TODO: evaluar más en detalle como manejar este error, por el momento retornamos -1
  if (!dictionary_has_key(tablas_de_paginas_segundo_nivel, string_itoa(numero_entrada_TP_segundo_nivel))) {
    return -1;
  } else {
    t_entrada_tabla_segundo_nivel* entrada_segundo_nivel = obtener_entrada_tabla_segundo_nivel(numero_tabla_paginas_segundo_nivel, numero_entrada_TP_segundo_nivel);
    int pid = obtener_pid_asignado_TP_segundo_nivel(numero_entrada_TP_segundo_nivel);

    if (tiene_marco_asignado_entrada_TP(entrada_segundo_nivel)) {
      marco = entrada_segundo_nivel->num_marco;

      xlog(COLOR_TAREA,
           "Se encontró el marco asignado a la entrada solicitada (TP_2do_nivel=%d, numero_entrada=%d, pid=%d, "
           "numero_marco=%d)",
           numero_tabla_paginas_segundo_nivel,
           numero_entrada_TP_segundo_nivel,
           pid,
           marco);
    } else if (hay_marcos_libres_asignados_al_proceso(pid)) {
      xlog(COLOR_TAREA,
           "Buscando alguno de los marcos libre de los asignados al proceso... (pid=%d, cantidad_marcos_disponibles=%d)",
           pid,
           cantidad_marcos_libres_asignados_al_proceso(pid));

      marco = obtener_y_asignar_primer_marco_libre_asignado_al_proceso(pid, entrada_segundo_nivel);

      xlog(COLOR_TAREA,
           "Se obtuvo el primer marco libre para a la entrada solicitada (TP_2do_nivel=%d, numero_entrada=%d, "
           "numero_marco=%d)",
           numero_tabla_paginas_segundo_nivel,
           numero_entrada_TP_segundo_nivel,
           marco);

      // según los algoritmos de reemplazo (clock/clock modificado): si bit_de_uso == 0 && bit_de_presencia == 1,
      // entonces bit_de_uso=1 (se habilita de nuevo) podríamos siempre setearlo a 1, pero agregamos éste condicional
      // para recordar/relacionar con la teoría
      if (entrada_segundo_nivel->bit_uso == 0)
        entrada_segundo_nivel->bit_uso = 1;
    } else {
      // si no tiene marcos libres => ejecutar algoritmo de sustitución de páginas
      marco = obtener_y_asignar_marco_segun_algoritmo_de_reemplazo(pid, entrada_segundo_nivel);

      xlog(COLOR_TAREA,
           "Se aplicó algoritmo de reemplazo y se obtuvo un marco para a la entrada solicitada (algoritmo=%s, "
           "TP_2do_nivel=%d, "
           "numero_entrada=%d, "
           "numero_marco=%d)",
           obtener_algoritmo_reemplazo_por_config(),
           numero_tabla_paginas_segundo_nivel,
           numero_entrada_TP_segundo_nivel,
           marco);
    }
  }

  return marco;
}

// TODO: lógica repetida con obtener_primer_marco_libre y hay_marcos_libres_asignados_al_proceso
int cantidad_marcos_libres_asignados_al_proceso(int pid) {
  bool marco_libre_asignado_a_este_proceso(t_marco * marco) {
    return marco->pid == pid && marco->ocupado == 0;
  }

  return list_count_satisfying(tabla_marcos, (void*)marco_libre_asignado_a_este_proceso);
}

// TODO: lógica repetida con obtener_primer_marco_libre
bool hay_marcos_libres_asignados_al_proceso(int pid) {
  bool marco_libre_asignado_a_este_proceso(t_marco * marco) {
    return marco->pid == pid && marco->ocupado == 0;
  }

  return list_any_satisfy(tabla_marcos, (void*)marco_libre_asignado_a_este_proceso);
}

// TODO: validar lógica repetida
t_tabla_segundo_nivel* obtener_TP_segundo_nivel(int numero_TP_primer_nivel, int numero_entrada_TP_primer_nivel) {
  t_tabla_primer_nivel* TP_primer_nivel = dictionary_get(tablas_de_paginas_primer_nivel, string_itoa(numero_TP_primer_nivel));
  t_entrada_tabla_primer_nivel* entrada_primer_nivel = dictionary_get(TP_primer_nivel->entradas_primer_nivel, string_itoa(numero_entrada_TP_primer_nivel));

  t_tabla_segundo_nivel* TP_segundo_nivel = dictionary_get(tablas_de_paginas_segundo_nivel, string_itoa(entrada_primer_nivel->num_tabla_segundo_nivel));

  return TP_segundo_nivel;
}

// TODO: validar lógica repetida
int obtener_numero_TP_segundo_nivel(int numero_TP_primer_nivel, int numero_entrada_TP_primer_nivel) {
  t_tabla_primer_nivel* TP_primer_nivel = dictionary_get(tablas_de_paginas_primer_nivel, string_itoa(numero_TP_primer_nivel));
  t_entrada_tabla_primer_nivel* entrada_primer_nivel = dictionary_get(TP_primer_nivel->entradas_primer_nivel, string_itoa(numero_entrada_TP_primer_nivel));

  return entrada_primer_nivel->num_tabla_segundo_nivel;
}

void dividir_memoria_principal_en_marcos() {
  xlog(COLOR_TAREA,
       "Dividiendo la memoria en marcos (memoria_bytes=%d, tamanio_pagina_bytes=%d, cantidad_marcos_en_memoria=%d)",
       obtener_tamanio_memoria_por_config(),
       obtener_tamanio_pagina_por_config(),
       obtener_cantidad_marcos_en_memoria());

  for (int numero_marco = 0; numero_marco < obtener_cantidad_marcos_en_memoria(); numero_marco++) {
    t_marco* marco = malloc(sizeof(t_marco));
    marco->num_marco = numero_marco;
    marco->direccion = 0;
    marco->pid = -1; // porque no tiene un proceso asignado al principio

    // para simular un bitmap de marcos libres/ocupados
    marco->ocupado = 0;

    // para facilitar el algoritmo de reemplazo
    marco->apuntado_por_puntero_de_clock = false;
    marco->entrada_segundo_nivel = NULL;

    list_add(tabla_marcos, marco);
  }
}

int obtener_cantidad_entradas_por_tabla_por_config() {
  return config_get_int_value(config, "PAGINAS_POR_TABLA");
}

int obtener_tamanio_memoria_por_config() {
  return config_get_int_value(config, "TAM_MEMORIA");
}

int obtener_cantidad_marcos_por_proceso_por_config() {
  return config_get_int_value(config, "TAM_MEMORIA");
}

int obtener_tamanio_pagina_por_config() {
  return config_get_int_value(config, "TAM_PAGINA");
}

char* obtener_algoritmo_reemplazo_por_config() {
  return config_get_string_value(config, "ALGORITMO_REEMPLAZO");
}

bool algoritmo_reemplazo_cargado_es(char* algoritmo) {
  return strcmp(obtener_algoritmo_reemplazo_por_config(), algoritmo) == 0;
}

int obtener_cantidad_marcos_en_memoria() {
  return obtener_tamanio_memoria_por_config() / obtener_tamanio_pagina_por_config();
}

void inicializar_estructuras_de_este_proceso(uint32_t pid, int tam_proceso) {
  // TODO: validar el comentario de abajo
  /// ESTA FUNCION DEBE DEVOLVER EL NUM DE TABLA DE PRIMER NIVEL ASIGNADA
  xlog(COLOR_TAREA, "Inicializando estructuras en memoria para un proceso (pid=%d, tamanio_bytes=%d)", pid, tam_proceso);

  t_tabla_primer_nivel* tabla_primer_nivel = tabla_paginas_primer_nivel_create();

  char* path_punto_monataje = config_get_string_value(config, "PATH_SWAP");
  inicializar_archivo_swap(pid, tam_proceso, path_punto_monataje);

  // agregamos una TP_primer_nivel en una estructura global
  dictionary_put(tablas_de_paginas_primer_nivel, string_itoa(tabla_primer_nivel->num_tabla), tabla_primer_nivel);

  xlog(COLOR_TAREA,
       "TP de primer nivel agregada a una estructura global (numero_TP=%d, cantidad_TP_primer_nivel=%d)",
       tabla_primer_nivel->num_tabla,
       dictionary_size(tablas_de_paginas_primer_nivel));
}

t_entrada_tabla_segundo_nivel* obtener_entrada_tabla_segundo_nivel(int numero_TP_segundo_nivel, int numero_entrada_TP_segundo_nivel) {
  t_tabla_segundo_nivel* TP_segundo_nivel = dictionary_get(tablas_de_paginas_segundo_nivel, string_itoa(numero_TP_segundo_nivel));
  t_entrada_tabla_segundo_nivel* entrada_segundo_nivel = dictionary_get(TP_segundo_nivel->entradas_segundo_nivel, string_itoa(numero_entrada_TP_segundo_nivel));

  return entrada_segundo_nivel;
}

int obtener_pid_asignado_TP_segundo_nivel(int numero_entrada_TP_segundo_nivel) {
  t_tabla_segundo_nivel* TP_segundo_nivel = dictionary_get(tablas_de_paginas_segundo_nivel, string_itoa(numero_entrada_TP_segundo_nivel));

  return TP_segundo_nivel->pid;
}

// TODO: validar
// TODO: lógica repetida con hay_marcos_disponibles_asignados_al_proceso
int obtener_y_asignar_primer_marco_libre_asignado_al_proceso(int pid, t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel) {
  int marco_libre_asignado_a_este_proceso(t_marco * marco) {
    return marco->pid == pid && marco->ocupado == 0;
  }

  t_marco* marco_libre = list_find(tabla_marcos, (void*)marco_libre_asignado_a_este_proceso);
  marco_libre->ocupado = 1;

  entrada_TP_segundo_nivel->num_marco = marco_libre->num_marco;

  // para facilitar el algoritmo de reemplazo
  marco_libre->entrada_segundo_nivel = entrada_TP_segundo_nivel;

  return marco_libre->num_marco;
}

void imprimir_marco(t_marco* marco) {
  xlog(COLOR_INFO,
       "[MARCO] numero=%d, pid=%d, ocupado=%s, numero_entrada_segundo_nivel=%d",
       marco->num_marco,
       marco->pid,
       (marco->ocupado) ? "SI" : "NO",
       (marco->entrada_segundo_nivel) ? marco->entrada_segundo_nivel->entrada_segundo_nivel : -1);
}

void algoritmo_reemplazo_imprimir_marco(t_marco* marco) {
  xlog(COLOR_INFO,
       "[Algoritmo Reemplazo] [MARCO] numero_marco=%d, pid=%d, ocupado=%s, numero_entrada_segundo_nivel=%d",
       marco->num_marco,
       marco->pid,
       (marco->ocupado) ? "SI" : "NO",
       (marco->entrada_segundo_nivel) ? marco->entrada_segundo_nivel->entrada_segundo_nivel : -1);
}

void mostrar_tabla_marcos() {
  xlog(COLOR_INFO, "Imprimiendo datos de los marcos en memoria...");

  list_iterate(tabla_marcos, (void*)imprimir_marco);
}

void algoritmo_reemplazo_imprimir_marcos_asignados(int pid) {
  xlog(COLOR_INFO, "[Algoritmo Reemplazo] Imprimiendo datos de los marcos asignados a un proceso... (pid=%d)", pid);

  t_list* marcos_asignados_al_proceso = obtener_marcos_asignados_a_este_proceso(pid);
  list_iterate(marcos_asignados_al_proceso, (void*)imprimir_marco);
}

void algoritmo_reemplazo_imprimir_entrada_segundo_nivel(t_entrada_tabla_segundo_nivel* entrada) {
  if (algoritmo_reemplazo_cargado_es("CLOCK")) {
    xlog(COLOR_INFO,
         "[Algoritmo Reemplazo] [ENTRADA] numero=%d, marco=%d, bit_de_uso=%d, bit_de_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_presencia);
  } else if (algoritmo_reemplazo_cargado_es("CLOCK-M")) {
    xlog(COLOR_INFO,
         "[Algoritmo Reemplazo] [ENTRADA] numero=%d, marco=%d, bit_de_uso=%d, bit_de_modificado=%d, "
         "bit_de_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_modif,
         entrada->bit_presencia);
  }
}

void imprimir_entrada_segundo_nivel(char* __, t_entrada_tabla_segundo_nivel* entrada) {
  if (algoritmo_reemplazo_cargado_es("CLOCK")) {
    xlog(COLOR_INFO,
         "....[TP_SEGUNDO_NIVEL] entrada_numero=%d, numero_marco=%d, bit_de_uso=%d, "
         "bit_de_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_presencia);
  } else if (algoritmo_reemplazo_cargado_es("CLOCK-M")) {
    xlog(COLOR_INFO,
         "....[TP_SEGUNDO_NIVEL] entrada_numero=%d, numero_marco=%d, bit_de_uso=%d, bit_de_modificado=%d, "
         "bit_de_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_modif,
         entrada->bit_presencia);
  }
}

void imprimir_entradas_tabla_paginas_segundo_nivel(t_tabla_segundo_nivel* tabla_segundo_nivel) {
  xlog(COLOR_INFO, "[TP_SEGUNDO_NIVEL] tp_numero=%d, cantidad_entradas=%d", tabla_segundo_nivel->num_tabla, dictionary_size(tabla_segundo_nivel->entradas_segundo_nivel));

  void imprimir_entrada_segundo_nivel(char* ___, t_entrada_tabla_segundo_nivel* entrada) {
    xlog(COLOR_INFO,
         "..[ENTRADA_SEGUNDO_NIVEL] numero=%d, marco=%d, bit_uso=%d, bit_modificado=%d, bit_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_modif,
         entrada->bit_presencia);
  }

  dictionary_iterator(tabla_segundo_nivel->entradas_segundo_nivel, (void*)imprimir_entrada_segundo_nivel);
}

void imprimir_tabla_paginas_primer_nivel(char* __, t_tabla_primer_nivel* tabla_primer_nivel) {
  xlog(COLOR_INFO,
       "[TP_PRIMER_NIVEL] tp_numero=%d, pid=%d, cantidad_entradas=%d",
       tabla_primer_nivel->num_tabla,
       tabla_primer_nivel->pid,
       dictionary_size(tabla_primer_nivel->entradas_primer_nivel));

  void imprimir_entrada_primer_segundo_nivel(char* ___, t_entrada_tabla_primer_nivel* entrada_primer_nivel) {
    t_tabla_segundo_nivel* tabla_segundo_nivel = obtener_TP_segundo_nivel(tabla_primer_nivel->num_tabla, entrada_primer_nivel->entrada_primer_nivel);

    xlog(COLOR_INFO,
         "..[ENTRADA_PRIMER_NIVEL] entrada_numero=%d, tp_segundo_nivel_numero=%d, pid=%d, cantidad_entradas=%d",
         entrada_primer_nivel->entrada_primer_nivel,
         tabla_segundo_nivel->num_tabla,
         tabla_segundo_nivel->pid,
         dictionary_size(tabla_segundo_nivel->entradas_segundo_nivel));

    dictionary_iterator(tabla_segundo_nivel->entradas_segundo_nivel, (void*)imprimir_entrada_segundo_nivel);
  }

  dictionary_iterator(tabla_primer_nivel->entradas_primer_nivel, (void*)imprimir_entrada_primer_segundo_nivel);
}

void imprimir_tablas_de_paginas() {
  xlog(COLOR_INFO, "Imprimiendo datos de las tablas de paginas...");

  dictionary_iterator(tablas_de_paginas_primer_nivel, (void*)imprimir_tabla_paginas_primer_nivel);
}


// llena el espacio en memoria con ceros
void llenar_memoria_mock() {
  int marco = 0, offset = 0;

  xlog(COLOR_TAREA, "Llenando los marcos de memoria con ceros..")

    while (marco < obtener_cantidad_marcos_en_memoria()) {
    memset(memoria_principal + offset, 0, obtener_tamanio_pagina_por_config());
    printf("marco=%d, %p + %d = %p", marco, memoria_principal, offset, memoria_principal + offset);

    char* datos_marco = mem_hexstring(memoria_principal + offset, obtener_tamanio_pagina_por_config());
    printf("%s\n\n", datos_marco);

    marco++;
    offset = offset + obtener_tamanio_pagina_por_config();
  }

  printf("\n");
}

// TODO: validar si aún se requiere
// se estaba usando para encontrar_marcos_asignados_por_procesos pero las tp_segundo_nivel ya tienen el pid
t_tabla_primer_nivel* obtener_tabla_paginas_primer_nivel_por_pid(int pid) {
  t_tabla_primer_nivel* tabla_paginas_primer_nivel = malloc(sizeof(t_tabla_primer_nivel));

  for (int cantidad_tablas_paginas_primer_nivel_leidas = 0; cantidad_tablas_paginas_primer_nivel_leidas < cantidad_tablas_paginas_primer_nivel();
       cantidad_tablas_paginas_primer_nivel_leidas++) {
    tabla_paginas_primer_nivel = dictionary_get(tablas_de_paginas_primer_nivel, string_itoa(cantidad_tablas_paginas_primer_nivel_leidas));

    if (tabla_paginas_primer_nivel->pid == pid)
      break;
  }

  return tabla_paginas_primer_nivel;
}

t_list* obtener_marcos_asignados_a_este_proceso(int pid) {
  bool marco_libre_asignado_a_este_proceso(t_marco * marco) {
    return marco->pid == pid;
  }

  bool marco_menor_numero(t_marco * marco_menor_numero, t_marco * marco_mayor_numero) {
    return marco_menor_numero->num_marco <= marco_mayor_numero->num_marco;
  }

  t_list* marcos_asignados = list_filter(tabla_marcos, (void*)marco_libre_asignado_a_este_proceso);

  // necesario mantener siempre el mismo orden, para mover el puntero del algoritmo de reemplazo en la cola circular
  t_list* marcos_asignados_ordenados_menor_a_mayor_numero = list_sorted(marcos_asignados, (void*)marco_menor_numero);

  return marcos_asignados_ordenados_menor_a_mayor_numero;
}

int cantidad_tablas_paginas_primer_nivel() {
  return dictionary_size(tablas_de_paginas_primer_nivel);
}

t_tabla_primer_nivel* tabla_paginas_primer_nivel_create() {
  t_tabla_primer_nivel* tabla_paginas_primer_nivel = malloc(sizeof(t_tabla_primer_nivel));

  xlog(COLOR_TAREA, "Creando TP de primer nivel...");
  // TODO: validar si conviene usar otra manera
  // contamos la cantidad de elementos en la estructura global (en el diccionario) y le sumamos uno
  tabla_paginas_primer_nivel->num_tabla = cantidad_tablas_paginas_primer_nivel() + 1;

  // TODO: evaluar porque se necesitaba el pid en la TP de 1er nivel
  tabla_paginas_primer_nivel->pid = -1;

  tabla_paginas_primer_nivel->entradas_primer_nivel = dictionary_create();

  for (int numero_entrada_primer_nivel = 0; numero_entrada_primer_nivel < obtener_cantidad_entradas_por_tabla_por_config(); numero_entrada_primer_nivel++) {
    t_entrada_tabla_primer_nivel* entrada_primer_nivel = malloc(sizeof(t_entrada_tabla_primer_nivel));

    // esto identifica cada entrada de TP 1er nivel, la MMU accede a ésta usando
    // floor(numero_pagina_DL/cant_entradas_por_tabla)
    entrada_primer_nivel->entrada_primer_nivel = numero_entrada_primer_nivel;

    // TODO: validar si se debe usar otro criterio para el numero_tabla_segundo_nivel
    t_tabla_segundo_nivel* tabla_paginas_segundo_nivel = tabla_paginas_segundo_nivel_create(numero_entrada_primer_nivel, 1);

    // agregamos una TP_segundo_nivel en una estructura global
    dictionary_put(tablas_de_paginas_segundo_nivel, string_itoa(tabla_paginas_segundo_nivel->num_tabla), tabla_paginas_segundo_nivel);
    xlog(COLOR_TAREA,
         "TP de segundo nivel agregada a una estructura global (numero_TP=%d, cantidad_TP_segundo_nivel=%d)",
         tabla_paginas_segundo_nivel->num_tabla,
         dictionary_size(tablas_de_paginas_segundo_nivel));


    entrada_primer_nivel->num_tabla_segundo_nivel = tabla_paginas_segundo_nivel->num_tabla;

    // agregamos una entrada_primer_nivel a la TP_primer_nivel
    dictionary_put(tabla_paginas_primer_nivel->entradas_primer_nivel, string_itoa(entrada_primer_nivel->entrada_primer_nivel), entrada_primer_nivel);
  }

  xlog(COLOR_TAREA,
       "TP de primer nivel creada con éxito (TP_1er_nivel_numero=%d, cantidad_entradas=%d)",
       tabla_paginas_primer_nivel->num_tabla,
       dictionary_size(tabla_paginas_primer_nivel->entradas_primer_nivel));

  return tabla_paginas_primer_nivel;
}

void inicializar_entrada_de_tabla_paginas(t_entrada_tabla_segundo_nivel* entrada_tabla_segundo_nivel) {
  // TODO: evaluar si siempre corresponde que se inicialize en 1, al menos para el clock y clock-m supongo que si
  entrada_tabla_segundo_nivel->bit_uso = 1;

  entrada_tabla_segundo_nivel->bit_modif = 0;

  // TODO: evaluar si corresponde que esté inicializado en 0
  entrada_tabla_segundo_nivel->bit_presencia = 0;

  entrada_tabla_segundo_nivel->num_marco = -1; // valor negativo porque no tiene un marco asignado
}

t_tabla_segundo_nivel* tabla_paginas_segundo_nivel_create(int numero_tabla_segundo_nivel, int pid) {
  t_tabla_segundo_nivel* tabla_paginas_segundo_nivel = malloc(sizeof(t_tabla_segundo_nivel));

  xlog(COLOR_TAREA, "Creando TP de segundo nivel... (numero_TP=%d)", numero_tabla_segundo_nivel);

  // TODO: esto debe coincidir con num_tabla_segundo_nivel que tiene la entrada de la TP de primer nivel
  tabla_paginas_segundo_nivel->num_tabla = numero_tabla_segundo_nivel;
  tabla_paginas_segundo_nivel->pid = pid;
  tabla_paginas_segundo_nivel->entradas_segundo_nivel = dictionary_create();

  for (int numero_entrada_segundo_nivel = 0; numero_entrada_segundo_nivel < obtener_cantidad_entradas_por_tabla_por_config(); numero_entrada_segundo_nivel++) {
    t_entrada_tabla_segundo_nivel* entrada_tabla_segundo_nivel = malloc(sizeof(t_entrada_tabla_segundo_nivel));
    entrada_tabla_segundo_nivel->entrada_segundo_nivel = numero_entrada_segundo_nivel;

    inicializar_entrada_de_tabla_paginas(entrada_tabla_segundo_nivel);

    dictionary_put(tabla_paginas_segundo_nivel->entradas_segundo_nivel, string_itoa(entrada_tabla_segundo_nivel->entrada_segundo_nivel), entrada_tabla_segundo_nivel);
  }

  xlog(COLOR_TAREA,
       "TP de segundo nivel creada con éxito (numero_TP=%d, cantidad_entradas=%d)",
       tabla_paginas_segundo_nivel->num_tabla,
       dictionary_size(tabla_paginas_segundo_nivel->entradas_segundo_nivel));

  return tabla_paginas_segundo_nivel;
}

int obtener_posicion_de_marco_del_listado(t_marco* marco, t_list* lista_marcos) {
  for (int posicion = 0; posicion < list_size(lista_marcos); posicion++) {
    if (marco == (t_marco*)list_get(lista_marcos, posicion)) {
      return posicion;
    }
  }

  return -1;
}

t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel_create(int num_entrada, int num_marco, int bit_uso, int bit_modif, int bit_presencia) {
  t_entrada_tabla_segundo_nivel* entrada = malloc(sizeof(t_entrada_tabla_segundo_nivel));

  entrada->entrada_segundo_nivel = num_entrada;
  entrada->num_marco = num_marco;
  entrada->bit_uso = bit_uso;
  entrada->bit_modif = bit_modif;
  entrada->bit_presencia = bit_presencia;

  return entrada;
}

t_marco* marco_create(int numero, int pid, t_estado_marco estado) {
  t_marco* marco = malloc(sizeof(t_marco));
  marco->pid = pid;
  marco->num_marco = numero;
  marco->ocupado = estado;

  return marco;
}

t_marco* obtener_marco_de_memoria(int numero_marco) {
  int es_este_marco(t_marco * marco) {
    return marco->num_marco == numero_marco;
  }

  t_marco* marco = list_find(tabla_marcos, (void*)es_este_marco);

  return marco;
}

// TODO: evaluar, que ocurre en el algoritmo de sustiticón..
// - y la anterior entrada que tenia asignada este marco?
// - dos o más entradas comparten el mismo marco pero éste marco ya no apunta a esa entrada..
void reasignar_marco(int numero_marco, int pid, t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel) {
  t_marco* marco = obtener_marco_de_memoria(numero_marco);

  marco->pid = pid;
  marco->ocupado = 1;

  entrada_TP_segundo_nivel->num_marco = numero_marco;

  // para facilitar el algoritmo de reemplazo
  marco->entrada_segundo_nivel = entrada_TP_segundo_nivel;
}

void algoritmo_clock_puntero_apuntar_al_marco(int numero_marco) {
  t_marco* marco = obtener_marco_de_memoria(numero_marco);
  marco->apuntado_por_puntero_de_clock = true;
}

void algoritmo_clock_entrada_imprimir_bits(t_entrada_tabla_segundo_nivel* entrada) {
  xlog(COLOR_INFO,
       "[Algoritmo Reemplazo] [ENTRADA] numero=%d, marco=%d, bit_uso=%d, bit_modificado=%d, bit_presencia=%d",
       entrada->entrada_segundo_nivel,
       entrada->num_marco,
       entrada->bit_uso,
       entrada->bit_modif,
       entrada->bit_presencia)
}
