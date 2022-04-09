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

  socket_servidor =
    socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

  if (socket_servidor != -1) {
    log_info(
      logger, "Se creo el socket con exito (socket=%d)", socket_servidor);
  }

  // con esto evitamos el bloqueo de conexion al matar el proceso forzosamente
  // con ctrl+c
  setsockopt(
    socket_servidor, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

  int status_bind =
    bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

  if (status_bind != -1) {
    log_info(logger,
             "Se asoció el puerto a la conexión con éxito (socket=%d)",
             socket_servidor);
  }

  int status_listen = listen(socket_servidor, SOMAXCONN);

  if (status_listen != -1) {
    log_info(
      logger, "Esperando conexiones entrantes por el puerto %s...", puerto);
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
  int cod_op;
  if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0) {
    log_info(logger,
             "Recibi una operacion (socket=%d, operacion=%d)",
             socket_cliente,
             cod_op);
    return cod_op;
  } else {
    log_info(logger,
             "Se cerró una de las conexiones entrantes (socket=%d)",
             socket_cliente);

    close(socket_cliente);
    return -1;
  }
}

// recv(socket, void* buffer, tamaño_buffer, flags)
void recibir_mensaje(int socket_cliente) {
  // validar_conexion();

  int size; // recv/4 requiere que la variable para guardar lo que recibamos sea
            // un (void*) ptr
  void* buffer = NULL;
  buffer = empty_buffer();

  recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);

  // 2. alocamos espacio para guardar los datos serializados que recibiremos
  // (la cant. de espacio se indicaba en el paquete recibido antes,
  // desreferenciamos size para obtener ese valor)
  buffer = malloc(size);

  // 3. recibimos los datos serializados y lo guardamos en `buffer`
  if (recv(socket_cliente, buffer, size, MSG_WAITALL) != -1) {
    log_info(logger,
             "Se recibió un mensaje (socket=%d, stream=%s)",
             socket_cliente,
             (char*)buffer);
  }


  free(buffer);
}

// 1. Usa recv(socket, void* buffer, tamaño_buffer, flags)
t_paquete* recibir_paquete(int socket_cliente) {
  // TODO: usar paquete_create() q crea un paquete con buffer vacío
  t_paquete* paquete = paquete_create();

  paquete->codigo_operacion = PAQUETE;

  // 1. recibimos el paquete->stream->size y lo guardamos en size
  recv(socket_cliente, &(paquete->buffer->size), sizeof(int), MSG_WAITALL);

  // 2. alocamos espacio para guardar los datos serializados que recibiremos
  // (la cant. de espacio se indicaba en el paquete recibido antes)
  paquete->buffer->stream = malloc(paquete->buffer->size);

  // 3. recibimos los datos serializados y lo guardamos en `buffer`
  if (recv(socket_cliente,
           paquete->buffer->stream,
           paquete->buffer->size,
           MSG_WAITALL) != -1) {
    log_info(logger,
             "Se recibió un paquete (socket=%d, buffer_bytes=%d)",
             socket_cliente,
             paquete->buffer->size);
  }


  return paquete;
}
