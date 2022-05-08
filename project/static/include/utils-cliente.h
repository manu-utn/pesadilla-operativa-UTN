#ifndef UTILS_CLIENTE_H_
#define UTILS_CLIENTE_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "libstatic.h"
#include "serializado.h"

//t_log *logger;

int conectar_a_servidor(char *ip, char *puerto);
int enviar(int socket_destino, t_paquete *paquete);
void enviar_mensaje(int socket_destino, t_paquete *paquete);
void enviar_paquete(int socket_destino, t_paquete *paquete);
void enviar_instrucciones(int socket_destino, t_paquete *paquete);
void enviar_pcb(int socket_destino, t_paquete *paquete);
void enviar_pcb_desalojado(int socket_destino, t_paquete *paquete);
void enviar_pcb_con_operacion_io(int socket_destino, t_paquete *paquete);
void terminar_cliente(int fd_servidor, t_log *logger, t_config *config);
#endif
