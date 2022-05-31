#include "sample.h"
#include "cpu.h"
#include "serializado.h"
#include "utils-cliente.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  // cargarConfiguracion();
  logger = iniciar_logger(DIR_LOG_MESSAGES, "CPU");
  config = iniciar_config(DIR_CPU_CFG);
  iniciar_tlb();
  setear_algoritmo_reemplazo();
  puntero_reemplazo = 0;
  mock_datos_tlb();

  /*pthread_t th1, th2;
  struct arg_struct args_dispatch;
  struct arg_struct args_interrupt;*/

  // t_config* config = iniciar_config(DIR_SERVIDOR_CFG);

  // pthread_create(&th1, NULL, (void*)escuchar_interrupt, NULL);
  estado_conexion_con_cliente = false;
  socket_memoria = conectarse_a_memoria();

  // HANDSHAKE CON MEMORIA
  /*t_mensaje_handshake_cpu_memoria* mensaje_hs =
  mensaje_handshake_create("MENSAJE PRUEBA");

  t_paquete* paquete_con_mensaje = paquete_create();

  paquete_add_mensaje_handshake(paquete_con_mensaje, mensaje_hs);

  enviar_mensaje_handshake(socket_memoria, paquete_con_mensaje);*/

  t_paquete *paquete = paquete_create();
  t_buffer *mensaje = crear_mensaje("Conexión aceptada por Kernel");
  paquete_cambiar_mensaje(paquete, mensaje);
  enviar_mensaje(socket_memoria, paquete);

  // free(mensaje);

  // paquete_destroy(paquete);

  pthread_t th, th2;
  pthread_create(&th, NULL, escuchar_dispatch, NULL), pthread_detach(th);
  pthread_create(&th2, NULL, iniciar_conexion_interrupt, NULL), pthread_detach(th2);

  log_info(logger, "Servidor listo para recibir al cliente Kernel");

  // free(config);
  // pthread_exit(0);

  pthread_exit(0);

  // TODO: esto debería estar en un hilo, para poder emular e iniciar la
  // también la conexion interrupt
}
