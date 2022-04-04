#include "utils-cliente.h"


void* serializar_paquete(t_paquete* paquete, int bytes) {
  void* magic = malloc(bytes);
  int desplazamiento = 0;

  memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
  desplazamiento += sizeof(int);
  memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
  desplazamiento += sizeof(int);
  memcpy(
    magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
  desplazamiento += paquete->buffer->size;

  return magic;
}

int conectar_a_servidor(char* ip, char* puerto) {
  log_info(logger, "Conectando a servidor... (ip=%s, puerto=%s)", ip, puerto);

  int status;
  struct addrinfo hints;
  struct addrinfo* server_info;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(ip, puerto, &hints, &server_info);

  int socket_cliente = socket(
    server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

  status =
    connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

  if (status != -1) {
    log_info(
      logger, "Conexión a servidor exitosa (ip=%s, puerto=%s)", ip, puerto);
  }

  freeaddrinfo(server_info);

  return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_destino) {
  log_info(logger,
           "Enviando mensaje... (socket_destino=%d, mensaje=%s)",
           socket_destino,
           mensaje);

  t_paquete* paquete = malloc(sizeof(t_paquete));

  paquete->codigo_operacion = MENSAJE;
  paquete->buffer = malloc(sizeof(t_buffer));
  paquete->buffer->size = strlen(mensaje) + 1;
  paquete->buffer->stream = malloc(paquete->buffer->size);
  memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

  int bytes = paquete->buffer->size + 2 * sizeof(int);

  void* a_enviar = serializar_paquete(paquete, bytes);

  int status_send = send(socket_destino, a_enviar, bytes, 0);

  if (status_send != -1) {
    log_info(
      logger, "Mensaje enviado con éxito (socket_destino=%d)", socket_destino);
  }

  free(a_enviar);
  eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete) {
  paquete->buffer = malloc(sizeof(t_buffer));
  paquete->buffer->size = 0;
  paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void) {
  log_info(logger, "Creando paquete..");

  t_paquete* paquete = malloc(sizeof(t_paquete));
  paquete->codigo_operacion = PAQUETE;
  crear_buffer(paquete);

  log_info(logger, "Paquete creado con éxito (codigo_operacion=%d)", PAQUETE);
  return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
  paquete->buffer->stream = realloc(
    paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

  memcpy(
    paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
  memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int),
         valor,
         tamanio);

  paquete->buffer->size += tamanio + sizeof(int);

  log_info(logger, "Paquete agregado con éxito (tamaño=%d) ", tamanio);
}

void enviar_paquete(t_paquete* paquete, int socket_destino) {
  log_info(logger, "Enviando paquete... (socket_destino=%d) ", socket_destino);

  int bytes = paquete->buffer->size + 2 * sizeof(int);
  void* a_enviar = serializar_paquete(paquete, bytes);

  int status = send(socket_destino, a_enviar, bytes, 0);

  if (status != -1) {
    log_info(logger,
             "Paquete enviado con éxito... (socket_destino=%d, tamaño=%d) ",
             socket_destino,
             bytes);
  }

  free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete) {
  free(paquete->buffer->stream);
  free(paquete->buffer);
  free(paquete);

  log_info(logger,
           "Se liberaron con éxito los recursos asignados durante de la "
           "creación del paquete");
}

void liberar_conexion(int socket_servidor) {
  close(socket_servidor);

  log_info(logger,
           "Se cerró la conexion establecida con éxito (socket=%d)",
           socket_servidor);
}
