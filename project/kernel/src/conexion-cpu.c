#include "planificador.h"

void enviar_interrupcion() {
  t_paquete *paquete = paquete_create();
  paquete->codigo_operacion = OPERACION_INTERRUPT;

  int socket_destino = conectarse_a_cpu("CPU_INTERRUPT");

  if (socket_destino != -1) {
    int status = enviar(socket_destino, paquete);
    paquete_destroy(paquete);

    if (status != -1) {
      xlog(COLOR_CONEXION, "La interrupción fue enviada con éxito (socket_destino=%d)", socket_destino);
      SE_ENVIO_INTERRUPCION = 1;
      close(socket_destino);
    }
  }
}

void iniciar_conexion_cpu_dispatch() {
  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexion_cpu_dispatch, NULL);
  pthread_detach(th);
}

void *escuchar_conexion_cpu_dispatch() {
  SOCKET_CONEXION_DISPATCH = conectarse_a_cpu("CPU_DISPATCH");

  CONEXION_ESTADO estado_conexion = CONEXION_ESCUCHANDO;
  xlog(COLOR_INFO, "Escuchando Conexión CPU Dispatch...");

  while (estado_conexion) {
    int codigo_operacion = recibir_operacion(SOCKET_CONEXION_DISPATCH);
    xlog(COLOR_PAQUETE, "Operación recibida (codigo=%d)", codigo_operacion);

    clock_gettime(CLOCK_REALTIME, &END);
    uint32_t tiempo_en_ejecucion = (END.tv_sec - BEGIN.tv_sec) * 1000 + (END.tv_nsec - BEGIN.tv_nsec) / 1000000;
    xlog(COLOR_INFO, "[TIMER]: Tiempo que pcb estuvo en cpu: %d milisegundos", tiempo_en_ejecucion);

    switch (codigo_operacion) {
      case OPERACION_PCB_CON_IO: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_DISPATCH);
        t_pcb *pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        xlog(COLOR_PAQUETE, "Se recibió un pcb con operación de I/O (pid=%d)", pcb->pid);
        xlog(COLOR_INFO, "Se bloquea un proceso (pid=%d, tiempo=%d)", pcb->pid, pcb->tiempo_de_bloqueado);

        pcb->tiempo_en_ejecucion += tiempo_en_ejecucion; // en milisegundos

        // FIX Basico para no calcular la estimacion en caso de FIFO
        if (algoritmo_cargado_es("SRT")) {
          pcb->estimacion_rafaga = calcular_estimacion_rafaga(pcb);
        }
        pcb->tiempo_en_ejecucion = 0;

        transicion_running_a_blocked(pcb);
        if (SE_ENVIO_INTERRUPCION) {
          sem_post(&HAY_PCB_DESALOJADO);
        } else {
          avisar_a_pcp_que_decida(); // Le indico al PCP q debe realizar una eleccion ya q cpu esta vacia
        }
        imprimir_pcb(pcb);
      } break;
      case OPERACION_PCB_CON_EXIT: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_DISPATCH);
        t_pcb *pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        xlog(COLOR_PAQUETE, "Se recibió un pcb con operación EXIT (pid=%d)", pcb->pid);
        xlog(COLOR_INFO, "Se finaliza un proceso (pid=%d)", pcb->pid);

        transicion_running_a_finished(pcb);
        if (SE_ENVIO_INTERRUPCION) {
          sem_post(&HAY_PCB_DESALOJADO);
        } else {
          avisar_a_pcp_que_decida(); // Le indico al PCP q debe realizar una eleccion ya q cpu esta vacia
        }

      } break;
      case OPERACION_PCB_DESALOJADO: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_DISPATCH);
        t_pcb *pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        pcb->tiempo_en_ejecucion += tiempo_en_ejecucion; // en milisegundos

        liberar_cpu();
        xlog(COLOR_PAQUETE, "Se recibió un pcb desalojado (pid=%d)", pcb->pid);

        imprimir_pcb(pcb);
        cambiar_estado_pcb(pcb, READY);

        agregar_pcb_a_cola(pcb, COLA_READY);

        sem_post(&HAY_PCB_DESALOJADO);
      } break;
      case -1: {
        xlog(COLOR_CONEXION, "Un proceso cliente se desconectó (socket=%d)", SOCKET_CONEXION_DISPATCH);

        // centinela para detener el loop del hilo asociado a la conexión entrante
        estado_conexion = CONEXION_FINALIZADA;
        break;
      }
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

        log_destroy(logger), close(SOCKET_CONEXION_DISPATCH);
        estado_conexion = CONEXION_FINALIZADA;
      } break;
      default: { xlog(COLOR_ERROR, "Operacion %d desconocida", codigo_operacion); } break;
    }
  }

  pthread_exit(NULL);
}
