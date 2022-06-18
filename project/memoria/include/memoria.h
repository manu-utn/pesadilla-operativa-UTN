#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <commons/collections/dictionary.h>
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

typedef enum {
  OPERACION_LECTURA = 1,
  OPERACION_ESCRITURA = 2
} t_operacion_memoria;

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
    t_dictionary* entradas_primer_nivel;
}t_tabla_primer_nivel;


typedef struct{
    int entrada_primer_nivel;
    int num_tabla_segundo_nivel;
}t_entrada_tabla_primer_nivel;

typedef struct{
    int num_tabla;
    int pid;
    t_dictionary* entradas_segundo_nivel;
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

// TODO: cambiar a tablas_TP_primer_nivel
// t_dictionary* diccionario_tablas;
t_dictionary* tablas_de_paginas_primer_nivel;

// t_list* lista_tablas_segundo_nivel;
t_dictionary* tablas_de_paginas_segundo_nivel;

t_list* marcos_prueba_clock;

void inicializar_estructuras_de_este_proceso(int pid, int tam_proceso);
void dividir_memoria_principal_en_marcos();
void mostrar_tabla_marcos();
void llenar_memoria_mock();

uint32_t buscar_dato_en_memoria(uint32_t dir_fisica);
int escribir_dato(uint32_t dir_fisica, uint32_t valor);

t_tabla_primer_nivel* tabla_paginas_primer_nivel_create();
t_tabla_segundo_nivel* tabla_paginas_segundo_nivel_create(int numero_tabla_segundo_nivel);
void inicializar_entrada_de_tabla_paginas(t_entrada_tabla_segundo_nivel* entrada_tabla_segundo_nivel);
int cantidad_tablas_paginas_primer_nivel();

int obtener_numero_TP_segundo_nivel(int numero_TP_primer_nivel, int entrada_tabla);
t_tabla_primer_nivel* obtener_tabla_paginas_primer_nivel_por_pid(int pid);
t_entrada_tabla_segundo_nivel* obtener_entrada_tabla_segundo_nivel(int numero_entrada_TP_segundo_nivel);
int obtener_pid_asignado_TP_segundo_nivel(int numero_entrada_TP_segundo_nivel);
bool tiene_marco_asignado_entrada_TP(t_entrada_tabla_segundo_nivel* entrada);

int obtener_marco(int num_tabla_segundo_nivel,int entrada_segundo_nivel);
int obtener_cantidad_marcos_en_memoria();
int obtener_y_asignar_primer_marco_libre_asignado_al_proceso(int pid, t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel);
bool hay_marcos_libres_asignados_al_proceso(int pid);

t_list* obtener_marcos_asignados_a_este_proceso(int pid);
int obtener_y_asignar_marco_segun_algoritmo_de_reemplazo(int pid, t_entrada_tabla_segundo_nivel* entrada_segundo_nivel_solicitada_para_acceder);
t_entrada_tabla_segundo_nivel* ejecutar_clock(t_list* marcos, t_entrada_tabla_segundo_nivel* entrada);
int entrada_TP_segundo_nivel_marco_asignado(int num_tabla_segundo_nivel, int entrada_segundo_nivel);

void liberar_estructuras_en_swap();

bool algoritmo_reemplazo_cargado_es(char* algoritmo);

// TODO: validar si deprecar, no se utiliza
int generar_numero_tabla();
void encontrar_marcos_en_tabla_segundo_nivel(int num_tabla_segundo_nivel, t_list* marcos);

// datos de la config
int obtener_cantidad_entradas_por_tabla_por_config();
int obtener_tamanio_memoria_por_config();
int obtener_cantidad_marcos_por_proceso_por_config();
char* obtener_algoritmo_reemplazo_por_config();
int obtener_tamanio_pagina_por_config();

#endif /* MEMORIA_H */
