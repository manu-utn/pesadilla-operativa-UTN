#include "utils-cliente.h"
#include "libstatic.h"

int conectar_a_servidor(char* ip, char* puerto) {
  xlog(COLOR_CONEXION, "Conectando a servidor... (ip=%s, puerto=%s)", ip, puerto);

  int status;
  struct addrinfo hints;
  struct addrinfo* server_info;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(ip, puerto, &hints, &server_info);

  int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

  status = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

  if (status == -1) {
    log_error(logger, "Ocurrió un error al intentar conectarse a un proceso servidor");
    return -1;
  } else {
    xlog(COLOR_CONEXION, "Conexión a servidor exitosa (ip=%s, puerto=%s)", ip, puerto);
  }

  freeaddrinfo(server_info);

  return socket_cliente;
}

int enviar(int socket_destino, t_paquete* paquete) {
  int size_paquete = get_paquete_size(paquete);
  void* paquete_serializado = serializar_paquete(paquete); // TODO: need free (1)

  int status = send(socket_destino, paquete_serializado, size_paquete, 0);

  free(paquete_serializado);

  return status;
}

void enviar_mensaje(int socket_destino, t_paquete* paquete) {
  paquete->codigo_operacion = OPERACION_MENSAJE;

  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    xlog(COLOR_PAQUETE,
         "Mensaje enviado con éxito (socket_destino=%d, stream_bytes=%d, "
         "stream=%s)",
         socket_destino,
         paquete->buffer->size,
         (char*)(paquete->buffer->stream));
  }
}

void enviar_instrucciones(int socket_destino, t_paquete* paquete) {
  paquete->codigo_operacion = OPERACION_CONSOLA;

  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    xlog(COLOR_PAQUETE,
         "Instrucciones enviadas con éxito (socket_destino=%d, buffer_bytes=%d)",
         socket_destino,
         paquete->buffer->size);
  }
}

void enviar_pcb_desalojado(int socket_destino, t_paquete* paquete) {
  paquete->codigo_operacion = OPERACION_PCB_DESALOJADO;

  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    xlog(COLOR_PAQUETE,
         "PCB desalojado fue enviado con éxito (socket_destino=%d, buffer_bytes=%d)",
         socket_destino,
         paquete->buffer->size);
  }
}

void enviar_pcb(int socket_destino, t_paquete* paquete) {
  paquete->codigo_operacion = OPERACION_PCB;

  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    xlog(COLOR_PAQUETE,
         "El PCB fue enviado con éxito (socket_destino=%d, buffer_bytes=%d)",
         socket_destino,
         paquete->buffer->size);
  }
}

void enviar_pcb_actualizado(int socket_destino, t_paquete* paquete) {
  paquete->codigo_operacion = PCB_ACTUALIZADO;

  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    log_info(logger,
             "El PCB fue enviado con éxito (socket_destino=%d, buffer_bytes=%d)",
             socket_destino,
             paquete->buffer->size);
  }
}

void enviar_mensaje_handshake(int socket_destino, t_paquete* paquete) {
  paquete->codigo_operacion = MENSAJE_HANDSHAKE;

  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    log_info(logger,
             "El MENSAJE fue enviado con éxito (socket_destino=%d, buffer_bytes=%d)",
             socket_destino,
             paquete->buffer->size);
  }
}

void enviar_operacion_read(int socket_destino, t_paquete* paquete) {
  paquete->codigo_operacion = READ;

  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    log_info(logger,
             "La operacion READ fue enviada con éxito (socket_destino=%d, buffer_bytes=%d)",
             socket_destino,
             paquete->buffer->size);
  }
}

// TODO: log_error si no asignó un codigo de operación
void enviar_paquete(int socket_destino, t_paquete* paquete) {
  int status = enviar(socket_destino, paquete);

  if (status != -1) {
    xlog(COLOR_PAQUETE,
         "Paquete enviado con éxito (socket_destino=%d, buffer_bytes=%d)",
         socket_destino,
         paquete->buffer->size);
  }
}

void terminar_cliente(int fd_servidor, t_log* logger, t_config* config) {
  close(fd_servidor);
  log_destroy(logger);
  config_destroy(config);
}
