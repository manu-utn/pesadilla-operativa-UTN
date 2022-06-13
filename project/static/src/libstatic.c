#include "libstatic.h"
#include "utils-servidor.h"

t_config* iniciar_config(char* config) {
  return config_create(config);
}

t_log* iniciar_logger(char* archivo, char* nombre) {
  return log_create(archivo, nombre, 1, LOG_LEVEL_INFO);
}

int get_paquete_size(t_paquete* paquete) {
  // tamaño de paquete->codigo_operacion + tamaño de paquete->buffer->stream +
  // tamaño de paquete->buffer->size
  return sizeof(int) + sizeof(int) + paquete->buffer->size;
}

void buffer_add(t_buffer* buffer, void* stream, int stream_size) {
  // memcpy(buffer->stream, );
}

t_buffer* empty_buffer() {
  t_buffer* nuevoBuffer = NULL;

  nuevoBuffer = malloc(sizeof(t_buffer));

  nuevoBuffer->size = 0;
  nuevoBuffer->stream = NULL;

  return nuevoBuffer;
}

void paquete_cambiar_mensaje(t_paquete* paquete, t_buffer* mensaje) {
  free(paquete->buffer);
  paquete->buffer = mensaje;
}

t_paquete* paquete_create() {
  t_paquete* nuevo_paquete = NULL;
  nuevo_paquete = malloc(sizeof(t_paquete));

  nuevo_paquete->buffer = NULL;
  nuevo_paquete->buffer = empty_buffer(); // TODO: need free()

  return nuevo_paquete; // TODO: need free()
}

t_buffer* crear_mensaje(char* texto) {
  int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  mensaje->size = mensaje_size;

  // memcpy(destino, fuente, cantidad_bytes)
  memcpy(mensaje->stream, (void*)texto, mensaje_size);

  return mensaje;
}

t_buffer* crear_mensaje_obtener_segunda_tabla(t_solicitud_segunda_tabla* read) {
  // int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes
  int mensaje_size = sizeof(int) * 3;
  int offset = 0;

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  // mensaje->size = mensaje_size;

  memcpy(mensaje->stream + offset, &(read->socket), sizeof(int));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->num_tabla_primer_nivel), sizeof(int));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->entrada_primer_nivel), sizeof(int));
  offset += sizeof(int);

  mensaje->size = offset;

  return mensaje;
}


t_buffer* crear_mensaje_respuesta_segunda_tabla(t_respuesta_solicitud_segunda_tabla* read) {
  // int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes
  int mensaje_size = sizeof(int) * 2;
  int offset = 0;

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  // mensaje->size = mensaje_size;
  memcpy(mensaje->stream + offset, &(read->socket), sizeof(int));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->num_tabla_segundo_nivel), sizeof(int));
  offset += sizeof(int);
  mensaje->size = offset;

  return mensaje;
}


t_buffer* crear_mensaje_obtener_marco(t_solicitud_marco* read) {
  // int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes
  int mensaje_size = sizeof(int) * 4;
  int offset = 0;

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  mensaje->size = mensaje_size;

  memcpy(mensaje->stream + offset, &(read->socket), sizeof(int));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->num_tabla_segundo_nivel), sizeof(int));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->entrada_segundo_nivel), sizeof(int));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->operacion), sizeof(int));
  offset += sizeof(int);

  return mensaje;
}

t_buffer* crear_mensaje_respuesta_marco(t_respuesta_solicitud_marco* read) {
  // int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes
  int mensaje_size = sizeof(int);
  int offset = 0;

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  mensaje->size = mensaje_size;

  memcpy(mensaje->stream + offset, &(read->num_marco), sizeof(int));
  offset += sizeof(int);

  return mensaje;
}

t_buffer* crear_mensaje_obtener_dato_fisico(t_solicitud_dato_fisico* read) {
  // int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes
  int mensaje_size = sizeof(int) * 2;
  int offset = 0;

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  mensaje->size = mensaje_size;

  memcpy(mensaje->stream + offset, &(read->socket), sizeof(int));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->dir_fisica), sizeof(int));
  offset += sizeof(int);
  return mensaje;
}

t_buffer* crear_mensaje_respuesta_dato_fisico(t_respuesta_dato_fisico* read) {
  // int mensaje_longitud = strlen(read->dato_buscado);                  // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = (sizeof(char) * mensaje_longitud) + sizeof(int); // 5 Bytes
  int mensaje_size = sizeof(int); // 5 Bytes

  // int mensaje_size = sizeof(int);
  int offset = 0;

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  mensaje->size = mensaje_size;

  // memcpy(mensaje->stream + offset, &mensaje_longitud, sizeof(int));
  // offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->dato_buscado), mensaje_size);
  offset += mensaje_size;
  return mensaje;
}


t_buffer* crear_mensaje_escritura_dato_fisico(t_escritura_dato_fisico* read) {
  // int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes
  int mensaje_size = sizeof(int) * 3;
  int offset = 0;
  // int size_valor = ((sizeof(char)) * (strlen(read->valor))) + 1;

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  mensaje->size = mensaje_size;

  memcpy(mensaje->stream + offset, &(read->socket), sizeof(int));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->dir_fisica), sizeof(uint32_t));
  offset += sizeof(int);
  memcpy(mensaje->stream + offset, &(read->valor), sizeof(uint32_t));
  offset += sizeof(uint32_t);
  return mensaje;
}

t_buffer* crear_mensaje_respuesta_escritura_dato_fisico(t_respuesta_escritura_dato_fisico* read) {
  // int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes
  int mensaje_size = sizeof(int);
  int offset = 0;

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer();               // <- generaba leaks
  mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  mensaje->size = mensaje_size;

  memcpy(mensaje->stream + offset, &(read->resultado), sizeof(int));
  offset += sizeof(int);

  return mensaje;
}

/*
t_buffer* crear_mensaje_pcb_actualizado(t_pcb* pcb, int tiempo_bloqueo) {
  // int mensaje_longitud = strlen(texto) + 1;           // sumamos el '\0' que indica fin de cadena
  // int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes

  t_buffer* mensaje = NULL;
  mensaje = empty_buffer(); // <- generaba leaks
  // mensaje->stream = malloc(mensaje_size); // TODO: need free (2)
  // mensaje->size = mensaje_size;

  int offset;
  int paquete_size = sizeof(int) * 6 + sizeof(t_pcb_estado);
  mensaje->stream = malloc(paquete_size);

  offset = 0, memcpy(mensaje->stream + offset, &(pcb->socket), sizeof(int));
  offset += sizeof(int), memcpy(mensaje->stream + offset, &(pcb->pid), sizeof(int));
  offset += sizeof(int), memcpy(mensaje->stream + offset, &(pcb->tamanio), sizeof(int));
  offset += sizeof(int), memcpy(mensaje->stream + offset, &(pcb->estimacion_rafaga), sizeof(int));
  offset += sizeof(int), memcpy(mensaje->stream + offset, &(pcb->tiempo_en_ejecucion), sizeof(int));
  offset += sizeof(int), memcpy(mensaje->stream + offset, &(pcb->program_counter), sizeof(int));
  offset += sizeof(int), memcpy(mensaje->stream + offset, &(pcb->estado), sizeof(t_pcb_estado));
  offset += sizeof(t_pcb_estado);

  mensaje->size = offset;
  for (int i = 0; i < list_size(pcb->instrucciones); i++) {
    t_instruccion* instruccion = list_get(pcb->instrucciones, i);

    int identificador_longitud = strlen(instruccion->identificador) + 1;
    int identificador_size = identificador_longitud * sizeof(char);

    int params_longitud = strlen(instruccion->params) + 1;
    int params_size = params_longitud * sizeof(char);

    int instruccion_size = identificador_size + params_size + sizeof(int) + sizeof(int);

    mensaje->stream = realloc(mensaje->stream, offset + instruccion_size);
    paquete_add_instruccion_pcb_actualizado(mensaje, instruccion);

    offset += instruccion_size;
  }

  if (tiempo_bloqueo != 0) {
    offset += sizeof(int), memcpy(mensaje->stream + offset, &tiempo_bloqueo, sizeof(int));
  }


  mensaje->size = offset;

  return mensaje;
}
*/

void paquete_add_instruccion_pcb_actualizado(t_buffer* mensaje, t_instruccion* instruccion) {
  int identificador_longitud = strlen(instruccion->identificador) + 1;
  int identificador_size = identificador_longitud * sizeof(char);

  int params_longitud = strlen(instruccion->params) + 1;
  int params_size = params_longitud * sizeof(char);

  int instruccion_size = identificador_size + params_size + sizeof(int) + sizeof(int);

  int offset = 0;

  if (mensaje->stream == NULL) {
    mensaje->stream = malloc(instruccion_size);
  } else {
    mensaje->stream = realloc(mensaje->stream, mensaje->size + instruccion_size);
    offset = mensaje->size;
  }

  memcpy(mensaje->stream + offset, &identificador_size, sizeof(int));

  offset += sizeof(int);
  memcpy(mensaje->stream + offset, instruccion->identificador, identificador_size);

  offset += identificador_size;
  memcpy(mensaje->stream + offset, &params_size, sizeof(int));

  offset += sizeof(int);
  memcpy(mensaje->stream + offset, instruccion->params, params_size);

  mensaje->size = mensaje->size + instruccion_size;

  offset += params_size;
}


void iterator_paquete(void* valor) {
  log_info(logger, "[PAQUETE] %s\n", (char*)valor);
}

void paquete_destroy(t_paquete* paquete) {
  int codigo_operacion = paquete->codigo_operacion;

  mensaje_destroy(paquete->buffer);
  free(paquete);

  xlog(COLOR_RECURSOS,
       "Se liberaron con éxito los recursos asignados durante de la creación del paquete (%d, tipo=%s)",
       codigo_operacion,
       obtener_tipo_operacion(codigo_operacion));
}

void instruccion_destroy(t_instruccion* instruccion) {
  free(instruccion->identificador);
  free(instruccion->params);
  free(instruccion);
}

void pcb_destroy(t_pcb* pcb) {
  list_destroy_and_destroy_elements(pcb->instrucciones, (void*)instruccion_destroy);
  free(pcb);
}

void operacion_read_destroy(t_operacion_read* read) {
  free(read);
}


void mensaje_destroy(t_buffer* mensaje) {
  free(mensaje->stream);
  free(mensaje);
}

void liberar_conexion(int socket) {
  close(socket);

  log_info(logger, "Se cerró la conexion con éxito (socket=%d)", socket);
}

void asignar_codigo_operacion(op_code codigo_operacion, t_paquete* paquete) {
  paquete->codigo_operacion = codigo_operacion;
}

void terminar_programa(int conexion, t_log* logger, t_config* config) {
  liberar_conexion(conexion), log_destroy(logger), config_destroy(config);
}

t_pcb* pcb_create(int socket, int pid, int tamanio) {
  t_pcb* pcb = NULL;

  pcb = malloc(sizeof(t_pcb));

  pcb->pid = pid;
  pcb->tamanio = tamanio;       // TODO: definir
  pcb->estimacion_rafaga = 0;   // TODO: definir
  pcb->tiempo_en_ejecucion = 0; // TODO: definir
  pcb->tiempo_de_bloqueado = 0; // TODO: definir
  pcb->program_counter = 0;     // TODO: definir
  pcb->estado = NEW;

  return pcb;
}

t_mensaje_handshake_cpu_memoria* mensaje_handshake_create(char* mensaje) {
  t_mensaje_handshake_cpu_memoria* mensaje_handshake = NULL;
  mensaje_handshake = malloc(sizeof(t_mensaje_handshake_cpu_memoria));

  mensaje_handshake->mensaje_handshake = mensaje;
  mensaje_handshake->size_mensaje = strlen(mensaje);

  return mensaje_handshake;
}

t_instruccion* instruccion_create(char* identificador, char* params) {
  t_instruccion* instruccion = malloc(sizeof(t_instruccion));

  int identificador_longitud = strlen(identificador) + 1;
  int identificador_size = sizeof(char) * identificador_longitud;

  int params_longitud = strlen(params) + 1;
  int params_size = sizeof(char) * params_longitud;

  instruccion->identificador = malloc(identificador_size);
  instruccion->params = malloc(params_size);

  memcpy(instruccion->identificador, (void*)identificador, identificador_size);
  memcpy(instruccion->params, params, params_size);

  return instruccion;
}

void imprimir_instruccion(t_instruccion* instruccion) {
  printf("identificador=%s, params=%s\n", instruccion->identificador, instruccion->params);
}

void imprimir_pcb(t_pcb* pcb) {
  printf("socket=%d, pid=%d, tamanio=%d, est_raf=%d, tiempo_en_ejecucion=%d, tiempo_en_bloqueado=%d, pc=%d, estado=%d, "
         "tabla=%d\n",
         pcb->socket,
         pcb->pid,
         pcb->tamanio,
         pcb->estimacion_rafaga,
         pcb->tiempo_en_ejecucion,
         pcb->tiempo_de_bloqueado,
         pcb->program_counter,
         pcb->estado,
         pcb->tabla_primer_nivel);

  printf("list_size=%d\n", list_size(pcb->instrucciones));

  for (int i = 0; i < list_size(pcb->instrucciones); i++) {
    printf("[INSTRUCCION]: ");
    t_instruccion* instruccion = list_get(pcb->instrucciones, i);
    imprimir_instruccion(instruccion);
  }
}

t_pcb* pcb_fake() {
  t_pcb* pcb = pcb_create(1, 10, 5);
  pcb->socket = 0;
  pcb->tamanio = 0;
  pcb->estimacion_rafaga = 0;
  pcb->program_counter = 0;
  pcb->tiempo_en_ejecucion = 0;
  pcb->tiempo_de_bloqueado = 0;

  return pcb;
}

void imprimir_instrucciones(t_list* lista) {
  for (int index = 0; index < list_size(lista); index++) {
    t_instruccion* instruccion = list_get(lista, index);
    imprimir_instruccion(instruccion);
  }
}

t_paquete* paquete_instruccion_create(int tamanio) {
  t_paquete* paquete = paquete_create();
  paquete->codigo_operacion = PAQUETE_INSTRUCCION;
  paquete->buffer->stream = malloc(tamanio);
  paquete->buffer->size = tamanio;

  return paquete;
}
