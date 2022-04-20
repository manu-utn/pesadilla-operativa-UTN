#include "utils-servidor.h"
#include "libstatic.h"

int iniciar_servidor(char* ip, char* puerto) {
  int socket_servidor;
  int optVal = 1;

  struct addrinfo hints, *servinfo;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(ip, puerto, &hints, &servinfo);

  socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

  if (socket_servidor != -1) {
    log_info(logger, "Se creo el socket con exito (socket=%d)", socket_servidor);
  }

  // con esto evitamos el bloqueo de conexion al matar el proceso forzosamente
  // con ctrl+c
  setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

  int status_bind = bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

  if (status_bind != -1) {
    log_info(logger, "Se asoció el puerto a la conexión con éxito (socket=%d)", socket_servidor);
  }

  int status_listen = listen(socket_servidor, SOMAXCONN);

  if (status_listen != -1) {
    log_info(logger, "Esperando conexiones entrantes por el puerto %s...", puerto);
  }

  freeaddrinfo(servinfo);

  return socket_servidor;
}

int esperar_cliente(int socket_servidor) {
  struct sockaddr dir_cliente;
  socklen_t tam_direccion = sizeof(struct sockaddr);

  int socket_cliente = accept(socket_servidor, &dir_cliente, &tam_direccion);

  log_info(logger, "Se conectó un cliente (socket=%d)", socket_cliente);

  return socket_cliente;
}

int recibir_operacion(int socket_cliente) {
  int cod_op = -1;
  if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0) {
    log_info(logger, "Recibi una operacion (socket=%d, operacion=%d)", socket_cliente, cod_op);
    return cod_op;
  } else {
    log_info(logger, "Se cerró una de las conexiones entrantes (socket=%d)", socket_cliente);

    close(socket_cliente);
    return -1;
  }
}

int recibir(int socket_cliente, t_buffer* buffer) {
  // 1. recibimos el paquete->stream->size y lo guardamos en size
  // recv(socket, void* buffer, tamaño_buffer, flags)
  recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

  // 2. alocamos espacio para guardar los datos serializados que recibiremos
  // (la cant. de espacio se indicaba en el paquete recibido antes)
  buffer->stream = malloc(buffer->size); // TODO: need free

  // 3. recibimos los datos serializados y lo guardamos en `buffer`
  int status = recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

  return status;
}

// recv(socket, void* buffer, tamaño_buffer, flags)
t_buffer* recibir_mensaje(int socket_cliente) {
  t_buffer* buffer = empty_buffer(); // TODO: need free x2
  int status = recibir(socket_cliente, buffer);

  if (status != -1) {
    log_info(logger,
             "Se recibió un mensaje (socket=%d, size=%d, stream=%s)",
             socket_cliente,
             buffer->size,
             (char*)buffer->stream);
  }

  return buffer;
}

t_paquete* recibir_paquete(int socket_cliente) {
  t_paquete* paquete = paquete_create(); // TODO: need free x3
  paquete->codigo_operacion = PAQUETE;

  int status = recibir(socket_cliente, paquete->buffer);

  // 3. recibimos los datos serializados y lo guardamos en `buffer`
  if (status != -1) {
    log_info(logger, "Se recibió un paquete (socket=%d, buffer_bytes=%d)", socket_cliente, paquete->buffer->size);
  }

  return paquete;
}

void terminar_servidor(int fd_servidor, t_log* logger, t_config* config) {
  log_destroy(logger);
  config_destroy(config);
  close(fd_servidor);
}
