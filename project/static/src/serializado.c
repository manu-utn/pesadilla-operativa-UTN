#include "serializado.h"

void* serializar_paquete(t_paquete* paquete) {
  // int size_paquete = sizeof(int) + sizeof(int) + paquete->buffer->size;
  int size_paquete = get_paquete_size(paquete);
  void* paquete_serializado = NULL;

  paquete_serializado = malloc(size_paquete); // TODO: need free (3)
  int offset = 0;
  memcpy(
    paquete_serializado + offset, &(paquete->codigo_operacion), sizeof(int));

  offset += sizeof(int);
  memcpy(paquete_serializado + offset, &(paquete->buffer->size), sizeof(int));

  offset += sizeof(int);
  memcpy(paquete_serializado + offset,
         paquete->buffer->stream,
         paquete->buffer->size);

  return paquete_serializado;
}

void** deserializar_paquete(t_paquete* paquete_serializado) {
  int offset, size_mensaje;
  void** mensajes = NULL;

  offset = 0;

  size_mensaje = 0;
  for (int index = 0, n = 1; offset < paquete_serializado->buffer->size;
       n++, index++) {
    mensajes = realloc(mensajes, sizeof(t_buffer) * n);

    t_buffer* mensaje = empty_buffer();

    size_mensaje = *(int*)(paquete_serializado->buffer->stream + offset);
    mensaje->size = size_mensaje;

    offset += sizeof(int);
    mensaje->stream = malloc(size_mensaje);
    memcpy(mensaje->stream,
           paquete_serializado->buffer->stream + offset,
           size_mensaje);

    mensajes[index] = (t_buffer*)mensaje;
    offset += size_mensaje;
    mensajes[index + 1] = NULL;
  }

  return mensajes;
}

void paquete_add_instruccion(t_paquete* paquete, t_instruccion* instruccion) {
  int identificador_longitud = strlen(instruccion->identificador) + 1;
  int identificador_size = identificador_longitud * sizeof(char);

  int params_longitud = strlen(instruccion->params) + 1;
  int params_size = params_longitud * sizeof(char);

  int instruccion_size =
    identificador_size + params_size + sizeof(int) + sizeof(int);

  int offset = 0;

  if (paquete->buffer->stream == NULL) {
    paquete->buffer->stream = malloc(instruccion_size);
  } else {
    paquete->buffer->stream = realloc(paquete->buffer->stream,
                                      paquete->buffer->size + instruccion_size);
    offset = paquete->buffer->size;
  }

  memcpy(paquete->buffer->stream + offset, &identificador_size, sizeof(int));

  offset += sizeof(int);
  memcpy(paquete->buffer->stream + offset,
         instruccion->identificador,
         identificador_size);

  offset += identificador_size;
  memcpy(paquete->buffer->stream + offset, &params_size, sizeof(int));

  offset += sizeof(int);
  memcpy(paquete->buffer->stream + offset, instruccion->params, params_size);

  paquete->buffer->size = paquete->buffer->size + instruccion_size;

  offset += params_size;
}

t_list* paquete_obtener_instrucciones(t_paquete* paquete_serializado) {
  int offset = 0;
  t_list* lista = list_create();

  while (offset < paquete_serializado->buffer->size) {
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    int identificador_size = 0, params_size = 0;

    identificador_size = *(int*)(paquete_serializado->buffer->stream + offset);
    instruccion->identificador = malloc(identificador_size);

    offset += sizeof(int);
    memcpy(instruccion->identificador,
           paquete_serializado->buffer->stream + offset,
           identificador_size);

    offset += identificador_size;
    params_size = *(int*)(paquete_serializado->buffer->stream + offset);
    instruccion->params = malloc(params_size);

    offset += sizeof(int);
    memcpy(instruccion->params,
           paquete_serializado->buffer->stream + offset,
           params_size);

    imprimir_instruccion(instruccion);

    list_add(lista, instruccion);

    offset += params_size;
  }

  return lista;
}
