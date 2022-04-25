#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <libstatic.h>
#include <semaphore.h>

#define DIR_BASE "/home/utnso/tp-2022-1c-Sisop-Oh-Yeah/project/"
#define MODULO "memoria"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_MEMORIA_CFG DIR_BASE MODULO "/config/configMemoria.cfg"

typedef struct{
    int puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    int paginas_por_tabla;
    int retardo_memoria;
    char* algoritmo_reemplazo;
    int marcos_por_proceso;
    int retardo_swap;
    char* path_swap;
}t_configuracion;



t_config * config;
t_log * logger;
uint32_t size_memoria_principal;

void* memoria_principal;


void limpiarConfiguracion();
int cargarConfiguracion();
int configValida(t_config* fd_configuracion);
void* escuchar_conexiones();
#endif /* MEMORIA_H */
