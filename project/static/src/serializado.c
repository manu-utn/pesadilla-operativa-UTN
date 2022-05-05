#include "serializado.h"
#include "libstatic.h"
#include <commons/collections/list.h>

void* serializar_paquete(t_paquete* paquete) {
  // int size_paquete = sizeof(int) + sizeof(int) + paquete->buffer->size;
  int size_paquete = get_paquete_size(paquete);
  void* paquete_serializado = NULL;

  paquete_serializado = malloc(size_paquete); // TODO: need free (3)
  int offset = 0;
  memcpy(paquete_serializado + offset, &(paquete->codigo_operacion), sizeof(int));

  offset += sizeof(int);
  memcpy(paquete_serializado + offset, &(paquete->buffer->size), sizeof(int));

  offset += sizeof(int);
  memcpy(paquete_serializado + offset, paquete->buffer->stream, paquete->buffer->size);

  return paquete_serializado;
}

t_list* deserializar_paquete(t_paquete* paquete_serializado) {
  int offset = 0, size_mensaje = 0;
  t_list* buffer = list_create();

  for (int index = 0, n = 1; offset < paquete_serializado->buffer->size; n++, index++) {
    t_buffer* mensaje = empty_buffer();

    size_mensaje = *(int*)(paquete_serializado->buffer->stream + offset);
    mensaje->size = size_mensaje;

    offset += sizeof(int);
    mensaje->stream = malloc(size_mensaje);
    memcpy(mensaje->stream, paquete_serializado->buffer->stream + offset, size_mensaje);

    list_add(buffer, (t_buffer*)mensaje);
    offset += size_mensaje;
  }

  return buffer;
}

void paquete_add_pcb(t_paquete* paquete, t_pcb* pcb) {
  int offset;
  int paquete_size = sizeof(int) * 5 + sizeof(t_pcb_estado);
  paquete->buffer->stream = malloc(paquete_size);

  offset = 0, memcpy(paquete->buffer->stream + offset, &(pcb->socket), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->pid), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->tamanio), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->estimacion_rafaga), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->program_counter), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->estado), sizeof(t_pcb_estado));
  offset += sizeof(t_pcb_estado);

  paquete->buffer->size = offset;
  for (int i = 0; i < list_size(pcb->instrucciones); i++) {
    t_instruccion* instruccion = list_get(pcb->instrucciones, i);

    int identificador_longitud = strlen(instruccion->identificador) + 1;
    int identificador_size = identificador_longitud * sizeof(char);

    int params_longitud = strlen(instruccion->params) + 1;
    int params_size = params_longitud * sizeof(char);

    int instruccion_size = identificador_size + params_size + sizeof(int) + sizeof(int);

    paquete->buffer->stream = realloc(paquete->buffer->stream, offset + instruccion_size);
    paquete_add_instruccion(paquete, instruccion);

    offset += instruccion_size;
  }
  paquete->buffer->size = offset;
}

void paquete_add_instruccion(t_paquete* paquete, t_instruccion* instruccion) {
  int identificador_longitud = strlen(instruccion->identificador) + 1;
  int identificador_size = identificador_longitud * sizeof(char);

  int params_longitud = strlen(instruccion->params) + 1;
  int params_size = params_longitud * sizeof(char);

  int instruccion_size = identificador_size + params_size + sizeof(int) + sizeof(int);

  int offset = 0;

  if (paquete->buffer->stream == NULL) {
    paquete->buffer->stream = malloc(instruccion_size);
  } else {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + instruccion_size);
    offset = paquete->buffer->size;
  }

  memcpy(paquete->buffer->stream + offset, &identificador_size, sizeof(int));

  offset += sizeof(int);
  memcpy(paquete->buffer->stream + offset, instruccion->identificador, identificador_size);

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
    memcpy(instruccion->identificador, paquete_serializado->buffer->stream + offset, identificador_size);

    offset += identificador_size;
    params_size = *(int*)(paquete_serializado->buffer->stream + offset);
    instruccion->params = malloc(params_size);

    offset += sizeof(int);
    memcpy(instruccion->params, paquete_serializado->buffer->stream + offset, params_size);

    list_add(lista, instruccion);

    offset += params_size;
  }

  return lista;
}

t_pcb* paquete_obtener_pcb(t_paquete* paquete_serializado) {
  int offset = 0;

  t_pcb* pcb = malloc(sizeof(t_pcb));

  memcpy(&(pcb->socket), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->pid), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->tamanio), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->estimacion_rafaga), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->program_counter), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->estado), paquete_serializado->buffer->stream + offset, sizeof(t_pcb_estado));

  offset += sizeof(t_pcb_estado);

  int instrucciones_size = paquete_serializado->buffer->size - offset;

  t_paquete* paquete_con_instrucciones = paquete_create();
  paquete_con_instrucciones->buffer->stream = malloc(instrucciones_size);
  paquete_con_instrucciones->buffer->size = instrucciones_size;

  memcpy(paquete_con_instrucciones->buffer->stream, paquete_serializado->buffer->stream + offset, instrucciones_size);

  pcb->instrucciones = paquete_obtener_instrucciones(paquete_con_instrucciones);

  paquete_destroy(paquete_con_instrucciones);

  return pcb;
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
    xlog(COLOR_SERIALIZADO,
         "Se agregó al buffer del paquete el tamaño de un stream (size=%d)",
         *(int*)(paquete->buffer->stream + offset));

    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, nuevo_mensaje->stream, nuevo_mensaje->size);
    xlog(COLOR_SERIALIZADO,
         "Se agregó al buffer del paquete un stream (stream=%s)",
         (char*)(paquete->buffer->stream + offset));

    paquete->buffer->size += mensaje_size;
  }
}
