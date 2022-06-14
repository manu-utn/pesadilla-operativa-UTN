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
    uint64_t timestamp;
}t_entrada_tlb;

typedef enum{
    ALGORITMO_FIFO=1,
    ALGORITMO_LRU=2
}algoritmo;

/*
typedef struct{
    int id_proceso;
    int tamanio_proceso;
    char* instrucciones;
    int pc;
    int nro_tabla_paginas;
    float estimacion_rafaga;
}t_pcb;*/

int algoritmo_reemplazo;
t_configuracion * configuracion;
t_config * fd_configuracion;
t_config* config;
t_log * logger;
t_list* tlb;
int socket_memoria;
bool estado_conexion_kernel;
bool estado_conexion_con_cliente;
int socket_cpu_dispatch;
int cargarConfiguracion();
int configValida(t_config* fd_configuracion);
void limpiarConfiguracion();
void* iniciar_conexion_interrupt();
void iniciar_ciclo_instruccion();
//void* escuchar_dispatch(void* arguments);
//void* escuchar_interrupt(void* arguments);
void* escuchar_dispatch();
void* escuchar_interrupt();
int conectarse_a_memoria();
t_instruccion* fetch(t_pcb* pcb);
int puntero_reemplazo;

void iniciar_tlb();
void ciclo_instruccion(t_pcb* pcb, int socket_cliente);
void decode(t_instruccion* instruccion, t_pcb* pcb, int socket_cliente);
void armar_operacion_read(t_operacion_read* read, t_instruccion* instruccion);

void* manejar_nueva_conexion(void* args);

void* escuchar_conexiones_entrantes_en_interrupt();

bool esta_en_tlb(int num_pagina);
void obtener_dato_fisico(t_solicitud_dato_fisico* solicitud_dato_fisico,
                         int num_marco,
                         int num_pagina,
                         int tam_pagina,
                         uint32_t dir_logica);

void obtener_numero_marco(t_solicitud_marco* solicitud_marco,
                          int num_pagina,
                          int cant_entradas_por_tabla,
                          int numero_tabla_segundo_nivel,
                          int operacion);

void obtener_numero_tabla_segundo_nivel(t_solicitud_segunda_tabla* read,
                                        t_pcb* pcb,
                                        int num_pagina,
                                        int cant_entradas_por_tabla);			


										
void armar_solicitud_tabla_segundo_nivel(t_solicitud_segunda_tabla* solicitud_tabla_segundo_nivel,
                                         int num_tabla_primer_nivel,
                                         int num_pagina,
                                         int cant_entradas_por_tabla);
void armar_solicitud_marco(t_solicitud_marco* solicitud_marco,
                           int num_pagina,
                           int cant_entradas_por_tabla,
                           int numero_tabla_segundo_nivel,
                           int operacion);		 

void escribir_dato_fisico(t_escritura_dato_fisico* escritura_dato_fisico,
                          int num_marco,
                          int num_pagina,
                          int tam_pagina,
                          uint32_t dir_logica,
                          uint32_t valor);

void armar_escritura_dato_fisico(t_escritura_dato_fisico* escritura_dato_fisico,
                                 int num_marco,
                                 int num_pagina,
                                 int tam_pagina,
                                 uint32_t dir_logica,
                                 uint32_t valor);


void armar_solicitud_dato_fisico(t_solicitud_dato_fisico* solicitud_dato_fisico,
                                 int num_marco,
                                 int num_pagina,
                                 int tam_pagina,
                                 uint32_t dir_logica);

void ejecutar_reemplazo();
void reemplazo_fifo();
void reemplazo_lru();
void mock_datos_tlb();
int buscar_marco_en_tlb(int num_pagina);
t_list* marcos;
t_list* paginas_en_memoria;
t_operacion_respuesta_fetch_operands* fetch_operands();

void execute_no_op();
void execute_io(t_pcb* pcb, t_instruccion* instruccion, int socket_cliente);
void execute_read_write(t_pcb* pcb,
                        int tam_pagina,
                        int cant_entradas_por_tabla,
                        int num_pagina,
                        uint32_t dir_logica,
                        uint32_t valor,
                        int operacion);
void execute_exit(t_pcb* pcb, int socket_cliente);

int instruccion_obtener_parametro(t_instruccion* instruccion, int numero_parametro);
void setear_algoritmo_reemplazo();
void check_interrupt(t_pcb* pcb, int socket_cliente);

#endif /* CPU_H_ */
