#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <libstatic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "consola.h"

t_config* config;

int main(int argc, char* argv[]) {
  logger = iniciar_logger(DIR_LOG_MESSAGES, "Consola");
  config = iniciar_config(DIR_CONSOLA_CFG);

  char path_archivo_instrucciones[100];
  int tamanio_proceso = 0;

  if (!argv[1]) {
    log_error(logger, "se requieren dos parámetros por la consola (ruta archivo de instrucciones + tamanio archivo)");
  } else {
    if (!argv[2]) {
      log_error(logger, "se requiere un segundo parámetro, el tamanio archivo)");
    }

    strcpy(path_archivo_instrucciones, argv[1]);

    tamanio_proceso = atoi(argv[2]);
  }

  // t_list* lista_instrucciones = obtener_instrucciones_de_archivo(DIR_CONFIG "/instrucciones.txt");
  t_list* lista_instrucciones = obtener_instrucciones_de_archivo(path_archivo_instrucciones);
  // t_pcb* pcb = pcb_fake();
  t_pcb* pcb = pcb_create(0, 0, tamanio_proceso);
  pcb->tamanio = tamanio_proceso; // TODO: debe ser información recibida por la terminal
  pcb->instrucciones = lista_instrucciones;

  t_paquete* paquete_con_pcb = paquete_create();
  paquete_add_pcb(paquete_con_pcb, pcb);

  int fd_kernel = conectarse_a_kernel();

  if (fd_kernel != -1) {
    enviar_pcb(fd_kernel, paquete_con_pcb);
  }

  pcb_destroy(pcb);
  paquete_destroy(paquete_con_pcb);

  escuchar_a_kernel(fd_kernel);

  return 0;
}

void escuchar_a_kernel(int socket_servidor) {
  CONEXION_ESTADO estado_conexion_con_servidor = CONEXION_ESCUCHANDO;

  while (estado_conexion_con_servidor) {
    xlog(COLOR_PAQUETE, "Esperando código de operación...");
    int codigo_operacion = recibir_operacion(socket_servidor);

    switch (codigo_operacion) {
      case OPERACION_MENSAJE: {
        recibir_mensaje(socket_servidor);
      } break;
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Finalizando ejecución...");

        // matar_proceso(socket_servidor);

        // liberar_conexion(socket_servidor), log_destroy(logger);
        terminar_programa(socket_servidor, logger, config);
        estado_conexion_con_servidor = CONEXION_FINALIZADA;
      } break;
      case -1: {
        xlog(COLOR_CONEXION, "el servidor se desconecto (socket=%d)", socket_servidor);

        liberar_conexion(socket_servidor);
        estado_conexion_con_servidor = CONEXION_FINALIZADA;
      } break;
    }
  }
  pthread_exit(NULL);
}

int conectarse_a_kernel() {
  char* ip = config_get_string_value(config, "IP_KERNEL");
  char* puerto = config_get_string_value(config, "PUERTO_KERNEL");
  int fd_servidor = conectar_a_servidor(ip, puerto);

  if (fd_servidor == -1) {
    log_error(logger, "No se pudo establecer la conexión con kernel, inicie el servidor kernel e intente nuevamente");

    return -1;
  }

  return fd_servidor;
}

t_list* obtener_instrucciones_de_archivo(char* ruta_archivo) {
  int buffer_tamanio = 255;
  char buffer[buffer_tamanio];
  FILE* archivo_con_instrucciones = fopen(ruta_archivo, "r");
  t_list* lista_instrucciones = list_create();

  while (fgets(buffer, buffer_tamanio, archivo_con_instrucciones)) {
    buffer[strcspn(buffer, "\n")] = 0; // removemos los saltos de linea

    char** instruccion_texto = string_n_split(buffer, 2, " ");
    char* identificador = instruccion_texto[0];
    char* params = instruccion_texto[1] ? instruccion_texto[1] : "";
    t_instruccion* instruccion;
    if (strcmp(identificador, "NO_OP") == 0) {
      char** texto_cantidad_no_op = string_split(params, " ");
      int cantidad_de_veces_no_op = atoi(texto_cantidad_no_op[0]);
      for (int i = 0; i < cantidad_de_veces_no_op; i++) {
        instruccion = instruccion_create(identificador, "");
        list_add(lista_instrucciones, instruccion);
      }
    } else {
      instruccion = instruccion_create(identificador, params);
      list_add(lista_instrucciones, instruccion);
    }

    // string_array_destroy(instruccion_texto);
  }

  fclose(archivo_con_instrucciones);

  return lista_instrucciones;
}
