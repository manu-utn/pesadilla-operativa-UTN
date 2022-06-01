#include "memoria.h"
#include "libstatic.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"

void* reservar_memoria_inicial(int size_memoria_total) {
  void* memoria_total = malloc(size_memoria_total);

  memset(memoria_total, 0, size_memoria_total);

  return memoria_total;
}

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
void* manejar_nueva_conexion(void* args) {
  int socket_cliente = *(int*)args;
  estado_conexion_con_cliente = true;
  while (estado_conexion_con_cliente) {
    int codigo_operacion = recibir_operacion(socket_cliente);

    switch (codigo_operacion) {
      case OPERACION_MENSAJE: {
        recibir_mensaje(socket_cliente);

        // t_paquete* paquete = recibir_paquete(cliente_fd);
        // t_mensaje_handshake_cpu_memoria* mensaje = paquete_obtener_mensaje_handshake(paquete);

      } break;
      case READ: {
        log_info(logger, "Comenzando operacion READ...");
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_solicitud_segunda_tabla* read = paquete_obtener_solicitud_tabla_segundo_nivel(paquete);

        log_info(logger, "Paquete recibido...");

        // PROCESO EL VALOR ENVIADO POR CPU, POR AHORA HARDCODEO UN VALOR PARA PROBAR LA CONEXION

        t_respuesta_solicitud_segunda_tabla* respuesta_read = malloc(sizeof(t_respuesta_solicitud_segunda_tabla));
        respuesta_read->num_tabla_segundo_nivel = 3;
        t_paquete* paquete_con_respuesta = paquete_create();
        paquete_add_respuesta_operacion_read(paquete_con_respuesta, respuesta_read);
        enviar_operacion_read(socket_cliente, paquete_con_respuesta);
        // DESCOMENTAR PARA RESOLVER SEG FAULT
        paquete_destroy(paquete_con_respuesta);

        free(respuesta_read);
      }
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

        log_destroy(logger), close(socket_memoria);
        // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
        estado_conexion_memoria = false;
        estado_conexion_con_cliente = false;
      }
      case OPERACION_OBTENER_SEGUNDA_TABLA: {
        xlog(COLOR_CONEXION, "Obteniendo numero de tabla de segundo nivel");
        // codigo_operacion = recibir_operacion(socket_memoria);
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_solicitud_segunda_tabla* req = malloc(sizeof(t_solicitud_segunda_tabla));

        req = obtener_solicitud_tabla_segundo_nivel(paquete);

        /// HACER LOS LLAMADOS A LOS METODOS CORRESPONDIENTES PARA OBTENER EL NUM DE TABLA

        t_paquete* paquete_respuesta = paquete_create();
        t_respuesta_solicitud_segunda_tabla* resp = malloc(sizeof(t_respuesta_solicitud_segunda_tabla));
        resp->socket = socket_cliente;
        resp->num_tabla_segundo_nivel = 200;
        t_buffer* mensaje = crear_mensaje_respuesta_segunda_tabla(resp);
        paquete_cambiar_mensaje(paquete_respuesta, mensaje),
          enviar_operacion_respuesta_segunda_tabla(socket_cliente, paquete_respuesta);

        paquete_destroy(paquete);
        break;
      }
      case OPERACION_OBTENER_MARCO: {
        xlog(COLOR_CONEXION, "Obteniendo numero de marco");
        // codigo_operacion = recibir_operacion(socket_memoria);
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_solicitud_marco* req = malloc(sizeof(t_solicitud_marco));

        req = obtener_solicitud_marco(paquete);

        /// HACER LOS LLAMADOS A LOS METODOS CORRESPONDIENTES PARA OBTENER EL NUM DE TABLA

        t_paquete* paquete_respuesta = paquete_create();
        t_respuesta_solicitud_marco* resp = malloc(sizeof(t_respuesta_solicitud_marco));
        resp->num_marco = 10;
        t_buffer* mensaje = crear_mensaje_respuesta_marco(resp);
        paquete_cambiar_mensaje(paquete_respuesta, mensaje),
          enviar_operacion_obtener_marco(socket_cliente, paquete_respuesta);
        break;
      }
      case OPERACION_OBTENER_DATO: {
        xlog(COLOR_CONEXION, "Obteniendo dato fisico en memoria");
        // codigo_operacion = recibir_operacion(socket_memoria);
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_solicitud_dato_fisico* req = malloc(sizeof(t_solicitud_dato_fisico));

        req = obtener_solicitud_marco(paquete);

        /// HACER LOS LLAMADOS A LOS METODOS CORRESPONDIENTES PARA OBTENER EL NUM DE TABLA

        t_paquete* paquete_respuesta = paquete_create();
        t_respuesta_dato_fisico* resp = malloc(sizeof(t_respuesta_dato_fisico));
        resp->size_dato = 6;
        resp->dato_buscado = malloc(7);
        memcpy(resp->dato_buscado, "holass", 7);
        memcpy(resp->dato_buscado + 5, "\0", 1);
        t_buffer* mensaje = crear_mensaje_respuesta_dato_fisico(resp);
        paquete_cambiar_mensaje(paquete_respuesta, mensaje),
          enviar_operacion_obtener_dato(socket_cliente, paquete_respuesta);
        break;
      }
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

int inicializar_tabla_marcos() {
  xlog(COLOR_CONEXION, "Inicializando tabla de marcos");
  int tam_tabla = 0;

  cant_marcos = (size_memoria_principal * 1024) / 256;

  while (tam_tabla < cant_marcos) {
    t_marco* marco = malloc(sizeof(t_marco));
    marco->num_marco = tam_tabla;
    marco->direccion = 0;
    marco->pid = 0;
    list_add(tabla_marcos, marco);
    tam_tabla++;
  }
  return tam_tabla;
}

void inicializar_proceso(int pid, int entradas_por_tabla) {
  xlog(COLOR_CONEXION, "Inicializando proceso");

  t_list* tabla_primer_nivel = list_create();
  t_list* tabla_segundo_nivel = list_create();

  for (int i = 0; i < entradas_por_tabla; i++) {
    t_pagina_primer_nivel* tabla_primer_nivel = malloc(sizeof(t_pagina_primer_nivel));
    tabla_primer_nivel->entrada_primer_nivel = i;
    for (int j = 0; j < entradas_por_tabla; j++) {
      t_pagina_segundo_nivel* tabla_segundo_nivel = malloc(sizeof(t_pagina_segundo_nivel));
      tabla_segundo_nivel->entrada_segundo_nivel = j;
      tabla_segundo_nivel->num_tabla = generar_numero_tabla();
      tabla_segundo_nivel->num_marco = buscar_marco_libre();
      tabla_segundo_nivel->bit_uso = 0;
      tabla_segundo_nivel->bit_modif = 0;
      tabla_segundo_nivel->bit_presencia = 0;
      tabla_primer_nivel->num_tabla_segundo_nivel = tabla_segundo_nivel->num_tabla;
      list_add(lista_tablas_segundo_nivel, tabla_segundo_nivel);
    }
  }
  dictionary_put(diccionario_paginas, string_itoa(pid), tabla_primer_nivel);
}

int generar_numero_tabla() {
  srand(time(NULL));
  int r = rand();
  return r;
}

int buscar_marco_libre() {
  int buscar_primer_libre(t_marco * marco) {
    return marco->pid == 0;
  }

  int marco_libre = (int)list_find(tabla_marcos, (void*)buscar_primer_libre);

  return marco_libre;
}


void mostrar_tabla_marcos() {
  xlog(COLOR_CONEXION, "########TABLA MARCOS################");


  for (int i = 0; i < list_size(tabla_marcos); i++) {
    t_marco* marco = (t_marco*)list_get(tabla_marcos, i);
    printf("Num Marco: %d\n", marco->num_marco);
    printf("Direccion: %d\n", marco->direccion);
    printf("PID: %d\n", marco->pid);
  }
}
