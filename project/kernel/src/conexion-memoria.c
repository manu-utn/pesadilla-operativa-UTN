#include "planificador.h"

int conectarse_a_memoria() {
  /* char *ip = config_get_string_value(config, "IP_MEMORIA"); */
  /* char *puerto = config_get_string_value(config, "PUERTO_MEMORIA"); */

  char *ip = obtener_ip_de_modulo_por_config("MEMORIA");
  char *puerto = obtener_puerto_de_modulo_por_config("MEMORIA");

  int fd_servidor = conectar_a_servidor(ip, puerto);

  if (fd_servidor == -1) {
    xlog(COLOR_ERROR,
         "No se pudo establecer la conexión con Memoria, inicie el servidor con %s e intente nuevamente",
         puerto);

    return -1;
  } else {
    xlog(COLOR_CONEXION, "Se conectó con éxito a Memoria a través de la conexión %s", puerto);
  }

  return fd_servidor;
}

void escuchar_conexion_con_memoria() {
  SOCKET_CONEXION_MEMORIA = conectarse_a_memoria();
  CONEXION_ESTADO estado_conexion_con_servidor = CONEXION_ESCUCHANDO;

  while (estado_conexion_con_servidor) {
    xlog(COLOR_PAQUETE, "Esperando código de operación de la conexión con Memoria...");
    int codigo_operacion = recibir_operacion(SOCKET_CONEXION_MEMORIA);

    switch (codigo_operacion) {
      case OPERACION_PROCESO_SUSPENDIDO_CONFIRMADO: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_MEMORIA);
        paquete_destroy(paquete);
        xlog(COLOR_CONEXION, "Se recibió confirmación de Memoria para suspender proceso");

        sem_post(&SUSPENSION_EXITOSA);
      } break;
      case OPERACION_ESTRUCTURAS_EN_MEMORIA_CONFIRMADO: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_MEMORIA);
        t_pcb *pcb = paquete_obtener_pcb(paquete);
        REFERENCIA_TABLA_RECIBIDA = pcb->tabla_primer_nivel;
        paquete_destroy(paquete);

        xlog(COLOR_CONEXION, "Se recibió confirmación de Memoria estructuras inicializadas para un proceso");

        pcb_destroy(pcb);

        sem_post(&INICIALIZACION_ESTRUCTURAS_EXITOSA);
      } break;
      case OPERACION_MENSAJE: {
        recibir_mensaje(SOCKET_CONEXION_MEMORIA);
      } break;
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Finalizando ejecución...");

        // matar_proceso(socket_servidor);
        // liberar_conexion(socket_servidor), log_destroy(logger);
        terminar_programa(SOCKET_CONEXION_MEMORIA, logger, config);
        estado_conexion_con_servidor = CONEXION_FINALIZADA;
      } break;
      case -1: {
        xlog(COLOR_CONEXION, "el servidor se desconecto (socket=%d)", SOCKET_CONEXION_MEMORIA);

        liberar_conexion(SOCKET_CONEXION_MEMORIA);
        estado_conexion_con_servidor = CONEXION_FINALIZADA;
      } break;
    }
  }
  pthread_exit(NULL);
}
