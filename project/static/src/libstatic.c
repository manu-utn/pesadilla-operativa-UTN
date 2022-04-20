#include "libstatic.h"
#include "utils-servidor.h"
#include <stdint.h>
#include <stdio.h>

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

void paquete_add_mensaje(t_paquete* paquete, t_buffer* nuevo_mensaje) {
  if (paquete->buffer == NULL) {
    paquete->buffer = nuevo_mensaje;
  } else {
    int mensaje_size = nuevo_mensaje->size + sizeof(int);
    int size = paquete->buffer->size + mensaje_size;

    paquete->buffer->stream = realloc(paquete->buffer->stream, size);

    int offset = 0;

    offset += paquete->buffer->size;
    memcpy(paquete->buffer->stream + offset, &(nuevo_mensaje->size), sizeof(int));

    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, nuevo_mensaje->stream, nuevo_mensaje->size);
    log_info(logger,
             "Se agregó con éxito mensaje al paquete (stream_bytes=%d, stream=%s)",
             nuevo_mensaje->size,
             (char*)(paquete->buffer->stream + offset));

    paquete->buffer->size += mensaje_size;
  }
}

void iterator_paquete(void* valor) {
  log_info(logger, "[PAQUETE] %s\n", (char*)valor);
}

void paquete_destroy(t_paquete* paquete) {
  mensaje_destroy(paquete->buffer);
  free(paquete);

  log_info(logger,
           "Se liberaron con éxito los recursos asignados durante de la "
           "creación del paquete");
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
  log_destroy(logger), config_destroy(config), liberar_conexion(conexion);
}

t_pcb* pcb_create(int socket, int pid, int tamanio) {
  t_pcb* pcb = NULL;

  pcb = malloc(sizeof(t_pcb));

  pcb->pid = pid;
  pcb->tamanio = tamanio;     // TODO: definir
  pcb->estimacion_rafaga = 0; // TODO: definir
  pcb->program_counter = 0;   // TODO: definir
  pcb->estado = NEW;

  return pcb;
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
  printf("socket=%d, pid=%d, tamanio=%d, est_raf=%d, pc=%d, estado=%d\n",
         pcb->socket,
         pcb->pid,
         pcb->tamanio,
         pcb->estimacion_rafaga,
         pcb->program_counter,
         pcb->estado);

  printf("list_size=%d\n", list_size(pcb->instrucciones));

  for (int i = 0; i < list_size(pcb->instrucciones); i++) {
    printf("instruccion\n");
    t_instruccion* instruccion = list_get(pcb->instrucciones, i);
    imprimir_instruccion(instruccion);
  }
}

t_pcb* pcb_fake() {
  t_pcb* pcb = pcb_create(1, 10, 5);
  pcb->socket = 4;
  pcb->tamanio = 5000;
  pcb->estimacion_rafaga = 2;
  pcb->program_counter = 1;

  return pcb;
}

void imprimir_instrucciones(t_list* lista) {
  for (int index = 0; index < list_size(lista); index++) {
    t_instruccion* instruccion = list_get(lista, index);
    imprimir_instruccion(instruccion);
  }
}
