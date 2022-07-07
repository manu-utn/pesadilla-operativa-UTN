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
#include <sys/stat.h>

#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>

#define MODULO "memoria"
#define DIR_LOG_MESSAGES DIR_BASE MODULO "/logs/messages.log"
#define DIR_MEMORIA_CFG DIR_BASE MODULO "/config/configMemoria.cfg"

typedef enum {
  OPERACION_LECTURA = 1,
  OPERACION_ESCRITURA = 2
} t_operacion_memoria;

typedef enum {
  CLOCK_MODIFICADO_NO_ES_VICTIMA = 0,
  CLOCK_MODIFICADO_VICTIMA_PRIORIDAD_ALTA = 1,
  CLOCK_MODIFICADO_VICTIMA_PRIORIDAD_MEDIA = 2,
  CLOCK_MODIFICADO_VICITMA_PRIORIDAD_BAJA = 3
} CLOCK_MODIFICADO_VICTIMA_NIVEL_PRIORIDAD;

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

typedef struct {
  int num_marco;
  uint32_t direccion;
  int pid;
  int ocupado;

  // para facilitar el algoritmo de reemplazo
  bool apuntado_por_puntero_de_clock;
  t_entrada_tabla_segundo_nivel* entrada_segundo_nivel;
} t_marco;

typedef struct {
  int marco;
  t_entrada_tabla_segundo_nivel* entrada;
} t_marco_asignado;

typedef struct {
  int num_tabla;
  int pid;
  t_dictionary* entradas_primer_nivel;
} t_tabla_primer_nivel;

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
t_tabla_segundo_nivel* tabla_paginas_segundo_nivel_create(int numero_tabla_segundo_nivel, int pid);
void inicializar_entrada_de_tabla_paginas(t_entrada_tabla_segundo_nivel* entrada_tabla_segundo_nivel);
int cantidad_tablas_paginas_primer_nivel();

int obtener_numero_TP_segundo_nivel(int numero_TP_primer_nivel, int entrada_tabla);
t_tabla_primer_nivel* obtener_tabla_paginas_primer_nivel_por_pid(int pid);
t_entrada_tabla_segundo_nivel* obtener_entrada_tabla_segundo_nivel(int numero_TP_segundo_nivel, int numero_entrada_TP_segundo_nivel);
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
t_entrada_tabla_segundo_nivel* entrada_victima_elegida_por_algoritmo_clock(t_list* marcos_asignados, t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder);
bool es_victima_segun_algoritmo_clock(t_entrada_tabla_segundo_nivel* entrada_elegida);
void algoritmo_clock_actualizar_puntero(t_marco* marco_seleccionado, t_marco* proximo_marco_seleccionado);
t_marco* algoritmo_clock_puntero_obtener_marco(t_list* lista_de_marcos);
int obtener_posicion_de_marco_del_listado(t_marco* marco, t_list* lista_marcos);

t_entrada_tabla_segundo_nivel* entrada_victima_elegida_por_algoritmo_clock_modificado(t_list* marcos_asignados, t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder);
bool es_victima_segun_algoritmo_clock_modificado(t_entrada_tabla_segundo_nivel* entrada_elegida);
CLOCK_MODIFICADO_VICTIMA_NIVEL_PRIORIDAD obtener_prioridad_victima_segun_algoritmo_clock_modificado(t_entrada_tabla_segundo_nivel* entrada_elegida);
t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel_create(int num_entrada, int num_marco, int bit_uso, int bit_modif, int bit_presencia);
void simular_solicitud_marco_por_mmu();
void imprimir_marco(t_marco* marco);
t_tabla_segundo_nivel* obtener_TP_segundo_nivel(int numero_TP_primer_nivel, int numero_entrada_TP_primer_nivel);
void imprimir_tablas_de_paginas();
void imprimir_tabla_paginas_primer_nivel(char* __, t_tabla_primer_nivel* tabla_primer_nivel);
void imprimir_entrada_segundo_nivel(char* __, t_entrada_tabla_segundo_nivel* entrada);
int cantidad_marcos_libres_asignados_al_proceso(int pid);
void algoritmo_reemplazo_imprimir_marco(t_marco* marco);
void algoritmo_reemplazo_imprimir_entrada_segundo_nivel(t_entrada_tabla_segundo_nivel* entrada);
void algoritmo_reemplazo_imprimir_marcos_asignados(int pid);

//SWAP
int crear_punto_de_montaje(char* path);
char* get_filepath(char* file, char* path, int pid);
void inicializar_archivo_swap(int pid, int tamanio, char* path);
void eliminar_archivo_swap(int pid, char* path);
void escribir_archivo_swap(char* filepath, void* datos, int numPagina, int tamanioPagina);

#endif /* MEMORIA_H */
