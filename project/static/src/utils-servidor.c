#include "utils-servidor.h"

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
      logger, "Esperando conexiones entrantes por el puerto %d...", puerto);
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

void* recibir_buffer(int* size, int socket_cliente) {
  void* buffer;

  recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
  buffer = malloc(*size);
  recv(socket_cliente, buffer, *size, MSG_WAITALL);

  return buffer;
}

void recibir_mensaje(int socket_cliente) {
  int size;
  char* buffer = recibir_buffer(&size, socket_cliente);

  log_info(logger,
           "Se recibió un mensaje (socket=%d, mensaje=%s)",
           socket_cliente,
           buffer);

  free(buffer);
}

t_list* recibir_paquete(int socket_cliente) {
  int size;
  int desplazamiento = 0;
  void* buffer;
  t_list* valores = list_create();
  int tamanio;

  buffer = recibir_buffer(&size, socket_cliente);
  while (desplazamiento < size) {
    memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    char* valor = malloc(tamanio);
    memcpy(valor, buffer + desplazamiento, tamanio);
    desplazamiento += tamanio;
    list_add(valores, valor);
  }
  free(buffer);
  return valores;
}
