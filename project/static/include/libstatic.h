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

typedef enum {
  INICIO = 0,
  MENSAJE = 1,
  PAQUETE = 2,
  CONSOLA = 3,
  PCB = 4,
  INTERRUPT = 5,
  MENSAJE_HANDSHAKE= 6,
  PCB_ACTUALIZADO=7,
  READ=8,
  FETCH=9
  } op_code;

typedef enum { CLIENTE_EXIT= 0, CLIENTE_RUNNING = 1} cliente_status;

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
  int socket;
  int pid;
  int tamanio;
  int estimacion_rafaga;
  int program_counter;
  int tabla_primer_nivel;
  t_pcb_estado estado;
  t_list* instrucciones;
} t_pcb;

typedef struct{
  int socket;
  int size_mensaje;
  char* mensaje_handshake;
}t_mensaje_handshake_cpu_memoria;


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
  void* dato_buscado;
}t_respuesta_dato_fisico;

t_config* iniciar_config(char*);
t_log* iniciar_logger(char* archivo, char* nombre);

t_buffer* crear_mensaje(char* texto);
t_pcb* pcb_create(int socket, int pid, int tamanio);
t_instruccion* instruccion_create(char* identificador, char* params);
t_paquete* paquete_create();
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
#endif
