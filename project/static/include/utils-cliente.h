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
void enviar_pcb_con_operacion_exit(int socket_destino, t_paquete* paquete);
void matar_proceso(int socket_conexion_entrante);
void terminar_cliente(int fd_servidor, t_log *logger, t_config *config);
void enviar_operacion_read(int socket_destino, t_paquete *paquete);
void enviar_mensaje_handshake(int socket_destino, t_paquete *paquete);
void enviar_pcb_interrupt(int socket_destino, t_paquete* paquete);
void enviar_operacion_obtener_dato(int socket_destino, t_paquete* paquete);
void enviar_operacion_obtener_segunda_tabla(int socket_destino, t_paquete* paquete);
void enviar_operacion_obtener_marco(int socket_destino, t_paquete* paquete);
void enviar_operacion_respuesta_segunda_tabla(int socket_destino, t_paquete* paquete);
void enviar_operacion_escribir_dato(int socket_destino, t_paquete* paquete);
void enviar_pcb_actualizado(int socket_destino, t_paquete* paquete);

void enviar_pcb_con_operacion_exit(int socket_destino, t_paquete* paquete);

void solicitar_suspension_de_proceso(int socket_destino, t_paquete *paquete);
void solicitar_inicializar_estructuras_en_memoria(int socket_destino, t_paquete *paquete);
void confirmar_suspension_de_proceso(int socket_destino, t_paquete *paquete);
void confirmar_estructuras_en_memoria(int socket_destino, t_paquete *paquete);
void solicitar_liberar_recursos_en_memoria_swap(int socket_destino, t_paquete *paquete);
#endif
