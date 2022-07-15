#ifndef CPU_H_
#define CPU_H_

#define MODULO "cpu"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_CPU_CFG DIR_BASE MODULO "/config/cpu.cfg"

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
#include <libstatic.h> // <-- STATIC LIB
#include "dir.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"

t_config    *   config;
t_log       *   logger;
int             socket_memoria;
bool            estado_conexion_kernel;
bool            estado_conexion_con_cliente;    
uint32_t        socket_cpu_dispatch;
uint32_t        HAY_PCB_PARA_EJECUTAR_;     //     = 0;
uint32_t        HAY_INTERRUPCION_; //              = 0;
uint32_t        CONEXION_CPU_INTERRUPT;
uint32_t        tamanio_pagina;
uint32_t        entradas_por_tabla;

void                realizar_handshake_memoria                  (void);
int                 conectarse_a_memoria                        (void);
void            *   escuchar_dispatch_                          (void);
void            *   manejar_nueva_conexion_                     (void           *   args);
void                ciclo_instruccion                           (t_pcb          *   pcb,                    uint32_t            socket_cliente);
t_instruccion   *   fetch                                       (t_pcb          *   pcb);       
uint32_t            decode                                      (t_instruccion  *   instruccion);       
uint32_t            fetch_operands                              (t_pcb          *   pcb,                    t_instruccion   *   instruccion);    
void                execute                                     (t_pcb          *   pcb,                    t_instruccion   *   instruccion,            uint32_t    socket_cliente,     uint32_t dato_leido_copy);
void                execute_no_op                               (void);     
void                execute_io                                  (t_pcb          *   pcb,                    t_instruccion   *   instruccion,            uint32_t    socket_cliente);  
void                execute_read                                (t_pcb          *   pcb,                    t_instruccion   *   instruccion);
void                execute_write                               (t_pcb          *   pcb,                    t_instruccion   *   instruccion);
void                execute_copy                                (t_pcb          *   pcb,                    t_instruccion   *   instruccion,            uint32_t dato_a_escribir);
void                execute_exit                                (t_pcb          *   pcb,                    int                 socket_cliente);
uint32_t            instruccion_obtener_parametro               (t_instruccion  *   instruccion,            uint32_t            numero_parametro);
void                check_interrupt                             (t_pcb          *   pcb,                    uint32_t            socket_cliente);
void            *   iniciar_conexion_interrupt                  (void);
void            *   escuchar_conexiones_entrantes_en_interrupt  (void);
uint32_t            obtener_tabla_segundo_nivel                 (uint32_t           tabla_primer_nivel,     uint32_t            entrada_primer_nivel);
uint32_t            obtener_marco_memoria                       (uint32_t           tabla_primer_nivel,     uint32_t            numero_pagina);
uint32_t            obtener_marco                               (uint32_t           tabla_segundo_nivel,    uint32_t            entrada_segundo_nivel);
uint32_t            obtener_dato_fisico                         (uint32_t           direccion_fisica);
uint32_t            obtener_direccion_fisica_memoria            (t_pcb*             pcb,                    t_instruccion   *   instruccion,            uint32_t    numero_parametro);
int                 escribir_dato_memoria                       (uint32_t           direccion_fisica,       uint32_t            dato_a_escribir);


//PARA PRUEBAS -> despues borrar
void                prueba_comunicacion_memoria                 (void);
#endif /* CPU_H_ */
