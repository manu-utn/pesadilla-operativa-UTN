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

typedef struct{
    int num_marco;
    uint32_t direccion;
    int pid;
    int ocupado;
}t_marco;


typedef struct{
    int num_tabla;
    int pid;
    t_list* entradas;
}t_tabla_primer_nivel;


typedef struct{
    int entrada_primer_nivel;
    int num_tabla_segundo_nivel;
}t_entrada_tabla_primer_nivel;

typedef struct{
    int num_tabla;
    int pid;
    t_list* entradas;
}t_tabla_segundo_nivel;

typedef struct{
    int entrada_segundo_nivel;
    int num_marco;
    int bit_uso;
    int bit_modif;
    int bit_presencia;
}t_entrada_tabla_segundo_nivel;

typedef struct{
    int marco;
    t_entrada_tabla_segundo_nivel* entrada;
}t_marco_asignado;

typedef struct{
    t_list* marcos;
    t_entrada_tabla_segundo_nivel* entrada_nueva;
}t_algoritmo_reemplazo;

t_config * config;
t_log * logger;
uint32_t size_memoria_principal;
void* memoria_principal;
int cant_marcos;
int tam_marcos;
bool estado_conexion_memoria;
bool estado_conexion_con_cliente;
int socket_memoria;
int puntero_clock;
int cant_marcos_por_proceso;
char* algoritmo_reemplazo;
void limpiarConfiguracion();
int cargarConfiguracion();
int configValida(t_config* fd_configuracion);
void* escuchar_conexiones();
void* reservar_memoria_inicial(int size_memoria_total);
void* manejar_nueva_conexion(void* args);

//Estructura admin de paginas y marcos
t_list* tabla_marcos;
t_dictionary* diccionario_tablas;
t_list* lista_tablas_segundo_nivel;
t_list* marcos_prueba_clock;
int buscar_marco_libre();
int generar_numero_tabla();
void inicializar_proceso(int pid, int entradas_por_tabla, int tam_proceso);
int inicializar_tabla_marcos();
void mostrar_tabla_marcos();
uint32_t buscar_dato_en_memoria(uint32_t dir_fisica);
int buscar_tabla_segundo_nivel(int num_tabla_primer_nivel,int entrada_primer_nivel);
int obtener_marco(int num_tabla_segundo_nivel,int entrada_segundo_nivel, int operacion);
void llenar_memoria_mock();
t_tabla_primer_nivel* encontrar_tabla(int pid);
void encontrar_marcos_en_tabla_segundo_nivel(int num_tabla_segundo_nivel, t_list* marcos);
t_list* encontrar_marcos_asignados_a_proceso(int pid);
int escribir_dato(uint32_t dir_fisica, uint32_t valor);
int ejecutar_reemplazo(int pid, t_entrada_tabla_segundo_nivel* entrada);
t_entrada_tabla_segundo_nivel* ejecutar_clock(t_list* marcos, t_entrada_tabla_segundo_nivel* entrada);

void liberar_estructuras_en_swap();
#endif /* MEMORIA_H */
