#include "kernel.h"
#include "libstatic.h"
#include "planificador.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include "xlog.h"
#include <commons/string.h>
#include <libstatic.h>
#include <stdio.h>
#include <string.h>

int SERVIDOR_KERNEL;

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "KERNEL");
  config = iniciar_config(DIR_SERVIDOR_CFG);
  PCBS_PROCESOS_ENTRANTES = queue_create(); // TODO: evaluar cuando liberar recursos
  sem_init(&HAY_PROCESOS_ENTRANTES, 0, 0);
  // pthread_mutex_init(&NO_HAY_PROCESOS_EN_SUSREADY, NULL);

  char* ip = config_get_string_value(config, "IP_KERNEL");
  char* puerto = config_get_string_value(config, "PUERTO_KERNEL");
  SERVIDOR_KERNEL = iniciar_servidor(ip, puerto);

  iniciar_planificacion();

  // esto lanza una excepción si la conexión interrupt de cpu no fue iniciada..
  // TODO: se debe usar cuando reciba una IO de CPU
  // enviar_interrupcion();

  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexiones_entrantes, NULL), pthread_detach(th);

  // necesario en vez de `return 0`, caso contrario el hilo main finalizará antes de los hilos detach
  pthread_exit(0);
}

int es_esta_instruccion(char* identificador, char** params) {
  return strcmp(identificador, params[0]) == 0;
}

int conectarse_a_cpu(char* conexion_puerto) {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, conexion_puerto);
  int fd_servidor = conectar_a_servidor(ip, puerto);

  if (fd_servidor == -1) {
    xlog(COLOR_ROJO,
         "No se pudo establecer la conexión con CPU, inicie el servidor con %s e intente nuevamente",
         conexion_puerto);

    return -1;
  }

  return fd_servidor;
}

void* escuchar_conexiones_entrantes() {
  while (1) {
    int cliente_fd = esperar_cliente(SERVIDOR_KERNEL);

    pthread_t th;
    pthread_create(&th, NULL, escuchar_nueva_conexion, &cliente_fd), pthread_detach(th);
  }

  pthread_exit(NULL);
}

void* escuchar_nueva_conexion(void* args) {
  int cliente_fd = *(int*)args;
  CONEXION_ESTADO estado_conexion = CONEXION_ESCUCHANDO;

  while (estado_conexion) {
    int codigo_operacion = recibir_operacion(cliente_fd);
    xlog(COLOR_VERDE, "Operación recibida (codigo=%d)", codigo_operacion);

    switch (codigo_operacion) {
      case PCB: {
        t_paquete* paquete = recibir_paquete(cliente_fd);
        t_pcb* pcb = paquete_obtener_pcb(paquete);

        // TODO: validar si necesitamos contemplar algo más
        queue_push(PCBS_PROCESOS_ENTRANTES, pcb);
        sem_post(&HAY_PROCESOS_ENTRANTES);

        // log_info(logger, "conexiones: pcbs=%d", queue_size(PCBS_PROCESOS_ENTRANTES));
        sem_post(&(COLA_NEW->instancias_disponibles));

        paquete_destroy(paquete);
        // pcb_destroy(pcb); // TODO: definir cuando liberar el recurso de pcb, supongo que al finalizar kernel (?)

        // descomentar para validar el memcheck
        // terminar_servidor(socket, logger, config);
        // return 0;
      } break;
      case -1: {
        xlog(COLOR_ROJO, "Un proceso cliente se desconectó (socket=%d)", cliente_fd);
        subir_grado_multiprogramacion();

        estado_conexion = CONEXION_FINALIZADA;
        break;
      }
      default:
        xlog(COLOR_ROJO, "Operacion desconocida");
        break;
    }
  }

  pthread_exit(NULL);
}
