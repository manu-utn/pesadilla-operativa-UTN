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



t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;


void limpiarConfiguracion();
int cargarConfiguracion();
int configValida(t_config* fd_configuracion);

#endif /* MEMORIA_H */
