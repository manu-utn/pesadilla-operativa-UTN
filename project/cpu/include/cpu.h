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
#include <stdarg.h>
#include <pthread.h>
#include <libstatic.h>

typedef struct {
	char* ip_escucha;
	int entradas_tlb;
	char* reemplazo_tlb;
	int reetardo_noop;
	char* ip_memoria;
	int puerto_memoria;
    int puerto_escucha_dispatch;
	int puerto_escucha_interrupt;
} t_configuracion;


struct arg_struct {
	char* arg1;
	char* arg2;
};

typedef struct{	
	int proceso;
	int pagina;
	int marco;
}t_entrada_tlb;


/*
typedef struct{
    int id_proceso;
    int tamanio_proceso;
    char* instrucciones;
    int pc;
    int nro_tabla_paginas;
    float estimacion_rafaga;
}t_pcb;*/

t_configuracion * configuracion;
t_config * fd_configuracion;
t_config* config;
t_log * logger;
t_list* tlb;
int socket_memoria;
bool estado_conexion_kernel;
bool estado_conexion_con_cliente;

int cargarConfiguracion();
int configValida(t_config* fd_configuracion);
void limpiarConfiguracion();
void iniciar_ciclo_instruccion();
//void* escuchar_dispatch(void* arguments);
//void* escuchar_interrupt(void* arguments);
void* escuchar_dispatch();
void* escuchar_interrupt();
int conectarse_a_memoria();
t_instruccion* fetch(t_pcb* pcb);

void iniciar_tlb();
void ciclo_instruccion(t_pcb* pcb);
void decode(t_instruccion* instruccion, t_pcb* pcb);
void armar_operacion_read(t_operacion_read* read, t_instruccion* instruccion);

void* manejar_nueva_conexion(void* args);
#endif /* CPU_H_ */
