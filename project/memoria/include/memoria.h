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
  MARCO_LIBRE = 0,
  MARCO_OCUPADO = 1,
} t_estado_marco;

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

typedef struct {
  int num_tabla;
  int pid;
  t_dictionary* entradas_primer_nivel;
} t_tabla_primer_nivel;

typedef struct{
    int entrada_primer_nivel;
    int num_tabla_segundo_nivel;

    // para facilitar el destroy del diccionario
    int num_tabla_primer_nivel;
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
    int numero_tabla_segundo_nivel;
}t_entrada_tabla_segundo_nivel;

typedef struct {
  int num_marco;
  int pid;
  int ocupado;

  // para facilitar el algoritmo de reemplazo
  bool apuntado_por_puntero_de_clock;
  int numero_tabla_segundo_nivel;
  t_entrada_tabla_segundo_nivel* entrada_segundo_nivel;
} t_marco;

typedef struct {
  int marco;
  t_entrada_tabla_segundo_nivel* entrada;
} t_marco_asignado;

typedef struct{
    t_list* marcos;
    t_entrada_tabla_segundo_nivel* entrada_nueva;
}t_algoritmo_reemplazo;

t_config              *   config;
t_log                 *   logger;

int                       ULTIMO_ID_1er_nivel;
int                       ULTIMO_ID_2do_nivel;

// MEMORIA PRINCIPAL
void                  *   memoria_principal;

uint32_t                  buscar_dato_en_memoria                    (uint32_t direccion_fisica);
uint32_t                  escribir_dato                             (uint32_t direccion_fisica,         uint32_t  valor);
uint32_t                  obtener_byte_inicio                       (uint32_t direccion_fisica,         uint32_t bit_modificado);
uint32_t                  obtener_marco_dato                        (uint32_t direccion_fisica);  
uint32_t                  obtener_offset_dato                       (uint32_t direccion_fisica);  
void                      actualizar_bits                           (uint32_t numero_marco,             uint32_t bit_modificado);
void                      dividir_memoria_principal_en_marcos();  
// FIN MEMORIA PRINCIPAL  

uint32_t                  inicializar_estructuras_de_este_proceso   (uint32_t pid,                      uint32_t tam_proceso);
t_tabla_primer_nivel  *   tabla_paginas_primer_nivel_create         (uint32_t pid);
t_tabla_segundo_nivel *   tabla_paginas_segundo_nivel_create        (uint32_t pid);
void                      inicializar_entrada_de_tabla_paginas      (t_entrada_tabla_segundo_nivel  *   entrada_tabla_segundo_nivel);
int                       obtener_numero_TP_segundo_nivel           (int numero_TP_primer_nivel,        int entrada_tabla);
int                       obtener_y_asignar_primer_marco_libre      (int pid,                           t_entrada_tabla_segundo_nivel  *   entrada_TP_segundo_nivel);
void                      liberar_memoria_asignada_a_proceso        (int pid);
void                      liberar_marco                             (t_marco* marco);

bool                      hay_marcos_libres_sin_superar_maximo_marcos_por_proceso(int pid);
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



void mostrar_tabla_marcos();
void llenar_memoria_mock();



int cantidad_tablas_paginas_primer_nivel();


t_tabla_primer_nivel* obtener_tabla_paginas_primer_nivel_por_pid(int pid);
t_entrada_tabla_segundo_nivel* obtener_entrada_tabla_segundo_nivel(int numero_TP_segundo_nivel, int numero_entrada_TP_segundo_nivel);
int obtener_pid_asignado_TP_segundo_nivel(int numero_entrada_TP_segundo_nivel);
bool tiene_marco_asignado_entrada_TP(t_entrada_tabla_segundo_nivel* entrada);

int obtener_marco(int num_tabla_segundo_nivel,int entrada_segundo_nivel);
int obtener_cantidad_marcos_en_memoria();
int obtener_y_asignar_primer_marco_libre_asignado_al_proceso(int pid, t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel);
bool hay_marcos_libres_asignados_al_proceso(int pid);

t_list* obtener_marcos_asignados_a_este_proceso(int pid);
int obtener_y_asignar_marco_segun_algoritmo_de_reemplazo(int pid, int numero_tabla_segundo_nivel, t_entrada_tabla_segundo_nivel* entrada_segundo_nivel_solicitada_para_acceder);
t_entrada_tabla_segundo_nivel* ejecutar_clock(t_list* marcos, t_entrada_tabla_segundo_nivel* entrada);
int entrada_TP_segundo_nivel_marco_asignado(int num_tabla_segundo_nivel, int entrada_segundo_nivel);

bool algoritmo_reemplazo_cargado_es(char* algoritmo);

// TODO: validar si deprecar, no se utiliza
int generar_numero_tabla();
void encontrar_marcos_en_tabla_segundo_nivel(int num_tabla_segundo_nivel, t_list* marcos);

// datos de la config
int obtener_cantidad_entradas_por_tabla_por_config();
int obtener_tamanio_memoria_por_config();
int obtener_cantidad_marcos_por_proceso_por_config();
char* obtener_algoritmo_reemplazo_por_config();
char* obtener_path_archivos_swap();
int obtener_tamanio_pagina_por_config();
int obtener_retardo_swap();

t_entrada_tabla_segundo_nivel* entrada_victima_elegida_por_algoritmo_clock(t_list* marcos_asignados, t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder);
bool es_victima_segun_algoritmo_clock(t_entrada_tabla_segundo_nivel* entrada_elegida);
void algoritmo_clock_actualizar_puntero(t_marco* marco_seleccionado, t_marco* proximo_marco_seleccionado);
t_marco* algoritmo_clock_puntero_obtener_marco(t_list* lista_de_marcos);
int obtener_posicion_de_marco_del_listado(t_marco* marco, t_list* lista_marcos);

t_entrada_tabla_segundo_nivel* entrada_victima_elegida_por_algoritmo_clock_modificado(t_list* marcos_asignados, t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder);
bool es_victima_segun_algoritmo_clock_modificado(t_entrada_tabla_segundo_nivel* entrada_elegida);
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
void entrada_asignada_a_marco_imprimir_bits(t_entrada_tabla_segundo_nivel* entrada);
int reemplazar_entrada_en_marco_de_memoria(t_entrada_tabla_segundo_nivel* entrada_victima, t_entrada_tabla_segundo_nivel* nueva_entrada);
void liberar_estructuras_en_memoria_de_este_proceso(int pid);
void tabla_paginas_primer_nivel_destroy(t_tabla_primer_nivel* tabla_paginas_primer_nivel);
void entrada_primer_nivel_destroy(t_entrada_tabla_primer_nivel* entrada_primer_nivel);
void entrada_segundo_nivel_destroy(t_entrada_tabla_segundo_nivel* entrada_segundo_nivel);

//SWAP
int crear_punto_de_montaje(char* path);
void asignar_marco_al_proceso(int pid, int numero_marco, t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel);
t_marco* marco_create(int numero, int pid, t_estado_marco estado);
t_marco* obtener_marco_de_memoria(int numero_marco);
void reasignar_marco(int numero_marco, int pid, t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel);
void algoritmo_clock_puntero_apuntar_al_marco(int numero_marco);
void algoritmo_clock_entrada_imprimir_bits(t_entrada_tabla_segundo_nivel* entrada);
void imprimir_entradas_tabla_paginas_segundo_nivel(t_tabla_segundo_nivel* tabla_segundo_nivel);
char* get_filepath(char* file, char* path, int pid);
void inicializar_archivo_swap(uint32_t pid, uint32_t tamanio);
void eliminar_archivo_swap(uint32_t pid);
void escribir_archivo_swap(char* filepath, void* datos, int numPagina);
char* leer_archivo_swap(char* filepath, int numPagina);
void escribir_datos_de_swap_en_marco(t_marco* marco);
void liberar_estructuras_en_swap(int pid);
void escribir_datos_de_marcos_en_swap(t_list* marcos);
void escribir_marco_en_swap(t_marco* marco);
bool marco_modificado(t_marco* marco);

#endif /* MEMORIA_H */
