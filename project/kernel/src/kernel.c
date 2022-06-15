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


CONEXION_ESTADO ESTADO_CONEXION_KERNEL;
int SERVIDOR_KERNEL;
sem_t CERRAR_PROCESO;
sem_t ASIGNAR_PID;
int ULTIMO_PID = 0;

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "kernel");
  config = iniciar_config(DIR_SERVIDOR_CFG);
  PCBS_PROCESOS_ENTRANTES = queue_create(); // TODO: evaluar cuando liberar recursos
  sem_init(&HAY_PROCESOS_ENTRANTES, 0, 0);
  sem_init(&CERRAR_PROCESO, 0, 0);
  sem_init(&ASIGNAR_PID, 0, 1);
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

  // TODO: remover cuando se solucione el problema del centinela global en loop que contiene a esperar_cliente()
  sem_wait(&CERRAR_PROCESO);

  // necesario en vez de `return 0`, caso contrario el hilo main finalizará antes de los hilos detach
  // pthread_exit(0);

  return 0;
}

int conectarse_a_cpu(char* conexion_puerto) {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, conexion_puerto);
  int fd_servidor = conectar_a_servidor(ip, puerto);

  if (fd_servidor == -1) {
    xlog(COLOR_ERROR,
         "No se pudo establecer la conexión con CPU, inicie el servidor con %s e intente nuevamente",
         conexion_puerto);

    return -1;
  } else {
    xlog(COLOR_CONEXION, "Se conectó con éxito a CPU a través de la conexión %s", conexion_puerto);
  }

  return fd_servidor;
}

void* escuchar_conexiones_entrantes() {
  ESTADO_CONEXION_KERNEL = CONEXION_ESCUCHANDO;

  while (ESTADO_CONEXION_KERNEL) {
    int cliente_fd = esperar_cliente(SERVIDOR_KERNEL);

    if (cliente_fd != -1) {
      t_paquete* paquete = paquete_create();
      t_buffer* mensaje = crear_mensaje("Conexión aceptada por Kernel");

      paquete_cambiar_mensaje(paquete, mensaje);
      enviar_mensaje(cliente_fd, paquete);
      paquete_destroy(paquete);
    }

    pthread_t th;
    pthread_create(&th, NULL, escuchar_nueva_conexion, &cliente_fd);
    pthread_detach(th);
  }

  pthread_exit(NULL);
}

void asignar_pid(t_pcb* pcb) {
  sem_wait(&ASIGNAR_PID);
  pcb->pid = ULTIMO_PID++;
  sem_post(&ASIGNAR_PID);
}

void asignar_estimacion_rafaga_inicial(t_pcb* pcb) {
  pcb->estimacion_rafaga = config_get_int_value(config, "ESTIMACION_INICIAL");
}

void* escuchar_nueva_conexion(void* args) {
  int socket_cliente = *(int*)args;
  CONEXION_ESTADO estado_conexion_con_cliente = CONEXION_ESCUCHANDO;

  while (estado_conexion_con_cliente) {
    int codigo_operacion = recibir_operacion(socket_cliente);
    xlog(COLOR_PAQUETE, "Operación recibida (codigo=%d)", codigo_operacion);

    switch (codigo_operacion) {
      case OPERACION_MENSAJE: {
        recibir_mensaje(socket_cliente);
      } break;
      case OPERACION_PCB: {
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_pcb* pcb = paquete_obtener_pcb(paquete);
        asignar_pid(pcb);
        // FIX Basico para no asignar la estimacion en caso de FIFO
        if (algoritmo_cargado_es("SRT")) {
          asignar_estimacion_rafaga_inicial(pcb);
        }
        pcb->socket = socket_cliente;
        // queue_push(PCBS_PROCESOS_ENTRANTES, pcb);

        // Se decidio realizar la transicion a new en esta instancia
        transicion_a_new(pcb);
        sem_post(&HAY_PROCESOS_ENTRANTES);
        // log_info(logger, "conexiones: pcbs=%d", queue_size(PCBS_PROCESOS_ENTRANTES));

        paquete_destroy(paquete);
        // pcb_destroy(pcb); // TODO: definir cuando liberar el recurso de pcb, supongo que al finalizar kernel (?)

        // descomentar para validar el memcheck
        // terminar_servidor(socket, logger, config);
        // return 0;
      } break;
      case -1: {
        xlog(COLOR_CONEXION, "Un proceso cliente se desconectó (socket=%d)", socket_cliente);

        // TODO: se debería actualizar el NEW
        // bajar_grado_multiprogramacion();
        liberar_espacio_en_memoria_para_proceso();

        // centinela para detener el loop del hilo asociado a la conexión entrante
        estado_conexion_con_cliente = CONEXION_FINALIZADA;

        close(socket_cliente);
        break;
      }
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

        log_destroy(logger), close(SERVIDOR_KERNEL);
        // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
        ESTADO_CONEXION_KERNEL = CONEXION_FINALIZADA;
        estado_conexion_con_cliente = CONEXION_FINALIZADA;

        sem_post(&CERRAR_PROCESO);
      } break;
      default: { xlog(COLOR_ERROR, "Operacion %d desconocida", codigo_operacion); } break;
    }
  }

  pthread_exit(NULL);
}
