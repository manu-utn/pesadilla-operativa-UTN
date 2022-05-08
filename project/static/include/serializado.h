#ifndef UTILS_SERIALIZADO_H_
#define UTILS_SERIALIZADO_H_

#include <commons/config.h>
#include <commons/log.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "libstatic.h"

//t_log *logger;

void *serializar_paquete(t_paquete *paquete);
t_list* deserializar_paquete(t_paquete *paquete_serializado);
void paquete_add_instruccion(t_paquete *paquete, t_instruccion *instruccion);
t_list *paquete_obtener_instrucciones(t_paquete *paquete_serializado);
void paquete_add_pcb(t_paquete *paquete, t_pcb *pcb);
t_pcb *paquete_obtener_pcb(t_paquete *paquete_serializado);
#endif
