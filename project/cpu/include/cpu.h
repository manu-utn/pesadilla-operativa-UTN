#ifndef CPU_H_
#define CPU_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



typedef struct {
	int entradas_tlb;
	char* reemplazo_tlb;
	int reetardo_noop;
	char* ip_memoria;
	int puerto_memoria;
    int puerto_escucha_dispatch;
	int puerto_escucha_interrupt;
} t_configuracion;


typedef struct{
    int id_proceso;
    int tamanio_proceso;
    char* instrucciones;
    int pc;
    int nro_tabla_paginas;
    float estimacion_rafaga;
}t_pcb;

t_configuracion * configuracion;
t_config * fd_configuracion;
t_log * logger;


int cargarConfiguracion();

void iniciar_ciclo_instruccion();


#endif /* CPU_H_ */
