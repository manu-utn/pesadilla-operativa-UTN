#include "libstatic.h"
#include "utils-servidor.h"
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

t_buffer* empty_buffer() {
  t_buffer* nuevoBuffer = NULL;

  nuevoBuffer = malloc(sizeof(t_buffer));

  nuevoBuffer->size = 0;
  nuevoBuffer->stream = NULL;

  return nuevoBuffer;
}

t_paquete* paquete_create() {
  t_paquete* nuevo_paquete = NULL;
  nuevo_paquete = malloc(sizeof(t_paquete));

  /*
    memset(nuevo_paquete, 0, sizeof(t_paquete)); // llenamos con ceros para leer
    mejor en el hex dump
  */
  nuevo_paquete->buffer = NULL;
  // nuevo_paquete->buffer = empty_buffer(); // TODO: need free()

  return nuevo_paquete; // TODO: need free()
}

t_buffer* crear_mensaje(char* texto) {
  /* char texto[4] = "hola"; // {'h', 'o', 'l', 'a', '\0'} */

  int mensaje_longitud =
    strlen(texto) + 1; // sumamos el '\0' que indica fin de cadena
  int mensaje_size = sizeof(char) * mensaje_longitud; // 5 Bytes

  t_buffer* mensaje = NULL;
  // mensaje = empty_buffer(); // <- generaba leaks
  mensaje = malloc(sizeof(t_buffer));
  mensaje->stream = NULL;
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
    // tamaño del stream del paquete + tamaño de nuevo_mensaje->stream + tamaño
    // de nuevo_mensaje->size el sizeof(int) es por nuevo_mensaje->size ya que
    // representa un entero
    int size = paquete->buffer->size + mensaje_size;

    // aumentamos el espacio en memoria para paquete->buffer->stream
    // al tamaño del bloque de memoria lo aumentamos tanto como el tamaño del
    // nuevo mensaje
    paquete->buffer->stream = realloc(paquete->buffer->stream, size);

    int offset = 0;

    // SERIALIZAMOS para agrupar los mensajes en paquete->buffer->stream
    //
    // nos desplazamos en bytes el valor de paquete->buffer->size para no pisar
    // lo último que tenga, luego de lo último agregaremos el
    // nuevo_mensaje->size
    offset += paquete->buffer->size;
    memcpy(
      paquete->buffer->stream + offset, &(nuevo_mensaje->size), sizeof(int));

    // nos desplazamos 4 bytes ó bien sizeof(int) para no pisar
    // nuevo_mensaje->size agregado con el anterior memcpy y agregamos
    // nuevo_mensaje->stream
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset,
           nuevo_mensaje->stream,
           nuevo_mensaje->size);
    log_info(
      logger,
      "Se agregó con éxito mensaje al paquete (stream_bytes=%d, stream=%s)",
      nuevo_mensaje->size,
      (char*)(paquete->buffer->stream + offset));

    // generamos efecto en el paquete, indicamos que su tamaño aumentó (porque
    // agregamos un nuevo mensaje)
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

void mensaje_destroy(t_buffer* mensaje) {
  free(mensaje->stream);
  free(mensaje);
}

void liberar_conexion(int socket) {
  close(socket);

  log_info(logger, "Se cerró la conexion con éxito (socket=%d)", socket);
}

void terminar_programa(int conexion, t_log* logger, t_config* config) {
  log_destroy(logger), config_destroy(config), liberar_conexion(conexion);
}
