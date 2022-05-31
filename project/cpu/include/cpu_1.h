#ifndef CPU_1_H_
#define CPU_1_H_

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

t_config    *   config;
t_log       *   logger;
uint32_t        socket_memoria;
bool            estado_conexion_kernel;
bool            estado_conexion_con_cliente;    
uint32_t        socket_cpu_dispatch;
uint32_t        HAY_PCB_PARA_EJECUTAR_          = 0;
uint32_t        HAY_INTERRUPCION_               = 0;
uint32_t        CONEXION_CPU_INTERRUPT;


//int                 main_                           (void);
void            *   escuchar_dispatch_                          (void);
void            *   manejar_nueva_conexion_                     (void           *   args);
void                ciclo_instruccion                           (t_pcb          *   pcb,            uint32_t            socket_cliente);
t_instruccion   *   fetch                                       (t_pcb          *   pcb);
uint32_t            decode                                      (t_instruccion  *   instruccion);
void                fetch_operands                              (void);
void                execute                                     (t_pcb          *   pcb,            t_instruccion   *   instruccion,        uint32_t    socket_cliente);
void                execute_no_op                               (uint32_t           cant_no_op);
void                execute_io                                  (t_pcb          *   pcb,            t_instruccion   *   instruccion,        uint32_t    socket_cliente);  
void                execute_exit                                (t_pcb          *   pcb,            uint32_t            socket_cliente);
uint32_t            instruccion_obtener_parametro               (t_instruccion  *   instruccion,    uint32_t            numero_parametro);
void                check_interrupt                             (t_pcb          *   pcb,            uint32_t            socket_cliente);
void            *   iniciar_conexion_interrupt                  (void);
void            *   escuchar_conexiones_entrantes_en_interrupt  (void);


#endif /* CPU_1_H_ */