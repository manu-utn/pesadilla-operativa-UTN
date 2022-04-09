#include "serializado.h"

void* serializar_paquete(t_paquete* paquete) {
  // int size_paquete = sizeof(int) + sizeof(int) + paquete->buffer->size;
  int size_paquete = get_paquete_size(paquete);
  void* paquete_serializado = NULL;

  paquete_serializado = malloc(size_paquete); // TODO: need free (3)

  // copiamos el paquete->codigo_operacion, por eso indicamos que el tamaño a
  // copiar es sizeof(int)
  int offset = 0;
  memcpy(
    paquete_serializado + offset, &(paquete->codigo_operacion), sizeof(int));

  // - copiamos el paquete->buffer->size, por eso indicamos que el tamaño a
  // copiar es sizeof(int)
  // - usamos el operador de dirección & porque memcpy se maneja con direcciones
  // de memoria
  // - nos desplazamos 4 bytes osea sizeof(int) para no pisar el
  // `codigo_operacion` copiado en el anterior memcpy
  offset += sizeof(int);
  memcpy(paquete_serializado + offset, &(paquete->buffer->size), sizeof(int));

  // copiamos el paquete->buffer->stream,por eso indicamos que el tamaño a
  // copiar es paquete->buffer->size
  // - nos desplazamos otros 4 bytes para no pisar el `paquete->buffer->size`
  // copiado en el anterior memcpy
  offset += sizeof(int);
  memcpy(paquete_serializado + offset,
         paquete->buffer->stream,
         paquete->buffer->size);

  return paquete_serializado;
}

void** deserializar_paquete(t_paquete* paquete_serializado) {
  int offset, size_mensaje;

  // lo tratamos como un arreglo de mensajes
  void** mensajes = NULL; // TODO: need free()

  offset = 0;

  // tamaño de cada mensaje dentro del paquete, varía según el contenido de cada mensaje
  size_mensaje = 0;
  for (int index = 0, n = 1; offset < paquete_serializado->buffer->size;
       n++, index++) {
    mensajes = realloc(mensajes, sizeof(t_buffer) * n);

    t_buffer* mensaje = empty_buffer();

    size_mensaje = *(int*)(paquete_serializado->buffer->stream + offset);
    mensaje->size = size_mensaje;

    // nos desplazamos 4 bytes, para no pisar el `buffer->size` que escribimos
    // con el anterior memcpy
    offset += sizeof(int);
    mensaje->stream = malloc(size_mensaje); // TODO: need free()
    memcpy(mensaje->stream,
           paquete_serializado->buffer->stream + offset,
           size_mensaje);

    // agregamos el mensaje como un elemento de un arreglo
    mensajes[index] = (t_buffer*)mensaje;

    // nos desplazamos buffer->size es decir el tamaño de buffer->stream
    // asi en la sig. iteración obtenemos el siguiente mensaje
    offset += size_mensaje;
  }

  return mensajes;
}
