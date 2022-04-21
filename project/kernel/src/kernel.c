#include "kernel.h"
#include "libstatic.h"
#include "planificador.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/string.h>
#include <libstatic.h>
#include <stdio.h>
#include <string.h>

int main() {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "KERNEL");
  config = iniciar_config(DIR_SERVIDOR_CFG);
  // esto lanza una excepción si la conexión interrupt de cpu no fue iniciada..
<<<<<<< HEAD
  // TODO: se debe usar cuando reciba una IO de CPU
  // enviar_interrupcion();

  iniciar_planificacion();

  // necesario en vez de `return 0`, caso contrario el hilo main finalizará antes de los hilos detach
  pthread_exit(0);
=======
  enviar_interrupcion();

  while (1) {
    int cliente_fd = esperar_cliente(socket);
    cliente_status cliente_estado = CLIENTE_RUNNING;

    while (cliente_estado) {
      int cod_op = recibir_operacion(cliente_fd);

      switch (cod_op) {
        case PCB: {
          t_paquete* paquete_con_pcb = recibir_paquete(cliente_fd);

          t_pcb* pcb_deserializado = paquete_obtener_pcb(paquete_con_pcb);
          imprimir_pcb(pcb_deserializado);

          // esto lanza una excepción si la conexión dispatch de cpu no fue iniciada..
          int socket_cpu_dispatch = conectarse_a_cpu("PUERTO_CPU_DISPATCH");

          if (socket_cpu_dispatch != -1) {
            enviar_pcb(socket_cpu_dispatch, paquete_con_pcb);
          }
          close(socket_cpu_dispatch);

          pcb_destroy(pcb_deserializado);
          paquete_destroy(paquete_con_pcb);

          // descomentar para validar el memcheck
          // terminar_servidor(socket, logger, config);
          // return 0;
        } break;
        case -1: {
          log_info(logger, "el cliente se desconecto");
          cliente_estado = CLIENTE_EXIT;
          break;
        }
        default:
          log_warning(logger, "Operacion desconocida. No quieras meter la pata");
          break;
      }
    }
  }
  
  /*
  t_pcb* pcb1 = pcb_fake(); // pid = 10, estimacion_rafaga = 2
  t_pcb* pcb2 = pcb_fake();
  pcb2->estimacion_rafaga = 1;
  pcb2->pid = 2;
  t_pcb* pcb3 = pcb_fake();
  pcb3->estimacion_rafaga = 1;
  pcb3->pid = 4;


  t_pcb* pcb_con_menor_rafaga = minimum_estimacion_rafaga(pcb1, pcb2); // Andando
  log_info(logger, "PID del pcb con menor estimacion de rafaga: %d", pcb_con_menor_rafaga->pid);

  t_cola_planificacion* cola_test = inicializar_cola();
  agregar_pcb_a_cola(pcb1, cola_test);
  agregar_pcb_a_cola(pcb2, cola_test);
  agregar_pcb_a_cola(pcb3, cola_test);
  log_info(logger, "Cantidad de elementos en cola: %d", list_size(cola_test->lista_pcbs));

  t_pcb* pcb_elegido_por_fifo = select_pcb_by_algorithm(cola_test, FIFO);
  log_info(logger, "PID del pcb elegido por FIFO: %d", pcb_elegido_por_fifo->pid);
  t_pcb* pcb_elegido_por_fifo2 = select_pcb_by_algorithm(cola_test, FIFO);
  log_info(logger, "PID del pcb elegido por FIFO: %d", pcb_elegido_por_fifo2->pid);
  t_pcb* pcb_elegido_por_fifo3 = select_pcb_by_algorithm(cola_test, FIFO);
  log_info(logger, "PID del pcb elegido por FIFO: %d", pcb_elegido_por_fifo3->pid);
  /*
  t_pcb* pcb_elegido_por_srt = select_pcb_by_algorithm(cola_test, SRT);
  log_info(logger, "PID del pcb elegido por SRT: %d", pcb_elegido_por_srt->pid);
  t_pcb* pcb_elegido_por_srt2 = select_pcb_by_algorithm(cola_test, SRT);
  log_info(logger, "PID del pcb elegido por SRT: %d", pcb_elegido_por_srt2->pid);
  t_pcb* pcb_elegido_por_srt3 = select_pcb_by_algorithm(cola_test, SRT);
  log_info(logger, "PID del pcb elegido por SRT: %d", pcb_elegido_por_srt3->pid);
  
  log_info(logger, "Cantidad de elementos en cola: %d", list_size(cola_test->lista_pcbs));
  /*
  free(pcb_elegido_por_srt);
  free(pcb_elegido_por_srt2);
  free(pcb_elegido_por_srt3);
  
  free(pcb_elegido_por_fifo);
  free(pcb_elegido_por_fifo2);
  free(pcb_elegido_por_fifo3);


  // free(pcb1);
  // free(pcb2);
  cola_destroy(cola_test);
  log_destroy(logger);
  config_destroy(config);
  */
  return 0;
>>>>>>> a797d173d9945fc3665c3575ff5ce59f0176a007
}

int es_esta_instruccion(char* identificador, char** params) {
  return strcmp(identificador, params[0]) == 0;
}

int conectarse_a_cpu(char* conexion_puerto) {
  char* ip = config_get_string_value(config, "IP_CPU");
  char* puerto = config_get_string_value(config, conexion_puerto);
  int fd_servidor = conectar_a_servidor(ip, puerto);

  if (fd_servidor == -1) {
    log_error(logger,
              "No se pudo establecer la conexión con CPU, inicie el servidor con %s e intente nuevamente",
              conexion_puerto);

    return -1;
  }

  return fd_servidor;
}

void enviar_interrupcion() {
  t_paquete* paquete = paquete_create();
  paquete->codigo_operacion = INTERRUPT;

  int socket_destino = conectarse_a_cpu("PUERTO_CPU_INTERRUPT");

  if (socket_destino != -1) {
    int status = enviar(socket_destino, paquete);

    if (status != -1) {
      log_info(logger, "La interrupcion fue enviada con éxito (socket_destino=%d)", socket_destino);

      close(socket_destino);
    }
  }
}
