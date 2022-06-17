#ifndef __MY_STATIC_LIB_H__
#define __MY_STATIC_LIB_H__

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "xlog.h"
#include "timer.h"
#include "dir.h"

typedef enum {
  OPERACION_EXIT = 0,
  MENSAJE = 1,
  PAQUETE = 2,
  CONSOLA = 3,
  PCB = 4,
  INTERRUPT = 5,
  MENSAJE_HANDSHAKE = 6,
  PCB_ACTUALIZADO2 = 7,
  READ = 8,
  FETCH = 9,
  // TODO: el resto deben ser removidos
  
  OPERACION_MENSAJE,
  OPERACION_PAQUETE,
  OPERACION_PCB,
  OPERACION_PCB_DESALOJADO,
  OPERACION_IO,
  OPERACION_OBTENER_SEGUNDA_TABLA,
  OPERACION_OBTENER_MARCO,
  OPERACION_OBTENER_DATO,
  OPERACION_BUSQUEDA_EN_MEMORIA_OK,
  OPERACION_INTERRUPT,
  OPERACION_CONSOLA,
  OPERACION_PCB_CON_IO,
  OPERACION_PCB_CON_EXIT,
  PAQUETE_INSTRUCCION,
  OPERACION_RESPUESTA_SEGUNDA_TABLA,
  OPERACION_RESPUESTA_MARCO,
  OPERACION_ESCRIBIR_DATO,
  OPERACION_PROCESO_SUSPENDIDO, OPERACION_PROCESO_SUSPENDIDO_CONFIRMADO,
  OPERACION_PROCESO_FINALIZADO,
  OPERACION_INICIALIZAR_ESTRUCTURAS, OPERACION_ESTRUCTURAS_EN_MEMORIA_CONFIRMADO
} op_code;

typedef enum { CONEXION_FINALIZADA = 0, CONEXION_ESCUCHANDO = 1 } CONEXION_ESTADO;

// TODO: pendiente de remover, hasta que cpu implemente CONEXION_ESTADO
typedef enum { CLIENTE_EXIT = 0, CLIENTE_RUNNING = 1 } cliente_status;

typedef struct {
  int size;
  void* stream;
} t_buffer;

typedef struct {
  op_code codigo_operacion;
  t_buffer* buffer;
} t_paquete;

// TODO: definir si se requiere esta abstracción
typedef struct{
  char* identificador;
  char* params;
} t_instruccion;

typedef enum {
  NEW,
  READY,
  EXEC,
  RUNNING,
  BLOCKED,
  SUSBLOCKED,
  SUSREADY,
  FINISHED
} t_pcb_estado;

// TODO: definir atributos: instrucciones y tabla de paginas
typedef struct {
  uint32_t socket;
  uint32_t pid;
  uint32_t tamanio;
  uint32_t estimacion_rafaga;
  uint32_t tiempo_en_ejecucion;
  uint32_t tiempo_de_bloqueado;
  uint32_t program_counter;
  uint32_t tabla_primer_nivel;
  t_pcb_estado estado;
  t_list* instrucciones;
} t_pcb;

typedef struct{
  int socket;
  int size_mensaje;
  char* mensaje_handshake;
}t_mensaje_handshake_cpu_memoria;

typedef struct{
  uint32_t dir_logica_origen;
}t_operacion_fetch_operands;

typedef struct{
  uint32_t valor;
}t_operacion_respuesta_fetch_operands;

typedef struct{
  int socket;
  uint32_t valor_buscado;
}t_respuesta_operacion_read;

typedef struct{
  int socket;
  uint32_t direccion_logica;
}t_operacion_read;

typedef struct{
	int socket;
	int num_tabla_primer_nivel;
	int entrada_primer_nivel;
}t_solicitud_segunda_tabla;

typedef struct{
  int socket;
	int num_tabla_segundo_nivel;
}t_respuesta_solicitud_segunda_tabla;

typedef struct{
	int socket;
	int num_tabla_segundo_nivel;
	int entrada_segundo_nivel;
  int operacion;// 1: lectura, 2: escritura
}t_solicitud_marco;

typedef struct{
	uint32_t num_marco;
}t_respuesta_solicitud_marco;

typedef struct{
	int socket;
	uint32_t dir_fisica;
}t_solicitud_dato_fisico;

typedef struct{
  int size_dato;
  uint32_t dato_buscado;
}t_respuesta_dato_fisico;

typedef struct{
	int socket;
	uint32_t dir_fisica;
  uint32_t valor;
}t_escritura_dato_fisico;

typedef struct{
  int resultado;
}t_respuesta_escritura_dato_fisico;

t_config* iniciar_config(char*);
t_log* iniciar_logger(char* archivo, char* nombre);

t_buffer* crear_mensaje(char* texto);
t_pcb* pcb_create(int socket, int pid, int tamanio);
t_instruccion* instruccion_create(char* identificador, char* params);
t_paquete* paquete_create();
t_paquete* paquete_instruccion_create(int tamanio);
t_buffer* empty_buffer();
int get_paquete_size(t_paquete* paquete);

void paquete_add_mensaje(t_paquete* paquete, t_buffer* nuevo_mensaje);

void mensaje_destroy(t_buffer* mensaje);
void paquete_destroy(t_paquete* paquete);
void instruccion_destroy(t_instruccion* instruccion);

void liberar_conexion(int socket);
void terminar_programa(int conexion, t_log* logger, t_config* config);
void paquete_cambiar_mensaje(t_paquete* paquete, t_buffer* mensaje);
void asignar_codigo_operacion(op_code codigo_operacion, t_paquete* paquete);

void imprimir_instruccion(t_instruccion* instruccion);
void imprimir_pcb(t_pcb* pcb);
void pcb_destroy(t_pcb* pcb);

t_mensaje_handshake_cpu_memoria* mensaje_handshake_create(char* mensaje);
void imprimir_instrucciones(t_list* lista);
t_pcb* pcb_fake();

// t_buffer* crear_mensaje_pcb_actualizado(t_pcb* pcb, int tiempo_bloqueo);
void paquete_add_instruccion_pcb_actualizado(t_buffer* mensaje, t_instruccion* instruccion);

t_buffer* crear_mensaje_obtener_segunda_tabla(t_solicitud_segunda_tabla* read);
t_buffer* crear_mensaje_obtener_marco(t_solicitud_marco* read);
t_buffer* crear_mensaje_obtener_dato_fisico(t_solicitud_dato_fisico* read);
t_buffer* crear_mensaje_respuesta_segunda_tabla(t_respuesta_solicitud_segunda_tabla* read);
t_buffer* crear_mensaje_respuesta_marco(t_respuesta_solicitud_marco* read);
t_buffer* crear_mensaje_respuesta_dato_fisico(t_respuesta_dato_fisico* read);

t_buffer* crear_mensaje_respuesta_escritura_dato_fisico(t_respuesta_escritura_dato_fisico* read);

t_buffer* crear_mensaje_escritura_dato_fisico(t_escritura_dato_fisico* read);
#endif
