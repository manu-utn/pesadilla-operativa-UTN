#include "planificador.h"
#include "kernel.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include "xlog.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/string.h>
#include <libstatic.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

t_pcb *PROCESO_EJECUTANDO = NULL;

int SOCKET_CONEXION_DISPATCH;
int SOCKET_CONEXION_MEMORIA;
// TODO: Evaluar si esta variable global es necesaria
// CONEXION_ESTADO ESTADO_CONEXION_DISPATCH;

// TODO: Evaluar si este semaforo es o no es necesario al final
// sem_t CONEXION_DISPATCH_DISPONIBLE; // semáforo binario
sem_t HAY_PCB_DESALOJADO;     // semáforo binario
sem_t EJECUTAR_ALGORITMO_PCP; // semaforo binario
sem_t MUTEX_BLOQUEO_SUSPENSION;
// time_t BEGIN;
// time_t END;
struct timespec BEGIN;
struct timespec END;
int SE_ENVIO_INTERRUPCION = 0;
int SE_INDICO_A_PCP_QUE_REPLANIFIQUE = 0;

void avisar_a_pcp_que_decida() {
  SE_INDICO_A_PCP_QUE_REPLANIFIQUE = 1;
  sem_post(&EJECUTAR_ALGORITMO_PCP);
}

void *escuchar_conexion_cpu_dispatch() {
  SOCKET_CONEXION_DISPATCH = conectarse_a_cpu("PUERTO_CPU_DISPATCH");

  CONEXION_ESTADO estado_conexion = CONEXION_ESCUCHANDO;
  xlog(COLOR_INFO, "Escuchando Conexión CPU Dispatch...");

  // sem_post(&CONEXION_DISPATCH_DISPONIBLE); // se sincroniza con el interrupt

  while (estado_conexion) {
    int codigo_operacion = recibir_operacion(SOCKET_CONEXION_DISPATCH);
    xlog(COLOR_PAQUETE, "Operación recibida (codigo=%d)", codigo_operacion);
    // END = time(NULL);
    clock_gettime(CLOCK_REALTIME, &END);
    // timer_detener();
    // timer_imprimir();
    int tiempo_en_ejecucion = (END.tv_sec - BEGIN.tv_sec) * 1000 + (END.tv_nsec - BEGIN.tv_nsec) / 1000000;
    xlog(COLOR_INFO,
         "[TIMER]: Tiempo que pcb estuvo en cpu: %d milisegundos",
         tiempo_en_ejecucion); // Timer en segundos para comparar aproximadamente con el de milisegundos

    switch (codigo_operacion) {
      case OPERACION_PCB_CON_IO: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_DISPATCH);
        t_pcb *pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        xlog(COLOR_PAQUETE, "Se recibió un pcb con operación de I/O (pid=%d)", pcb->pid);
        xlog(COLOR_INFO, "Se bloquea un proceso (pid=%d, tiempo=%d)", pcb->pid, pcb->tiempo_de_bloqueado);

        // pcb->tiempo_en_ejecucion += TIMER.tiempo_total; // en milisegundos
        pcb->tiempo_en_ejecucion += tiempo_en_ejecucion;
        pcb->estimacion_rafaga = calcular_estimacion_rafaga(pcb);
        pcb->tiempo_en_ejecucion = 0;


        // TODO: se debe bloquear el proceso, sin bloquear la escucha
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

        // pcb->tiempo_en_ejecucion += TIMER.tiempo_total; // en milisegundos
        pcb->tiempo_en_ejecucion += tiempo_en_ejecucion;

        liberar_cpu();
        xlog(COLOR_PAQUETE, "Se recibió un pcb desalojado (pid=%d)", pcb->pid);

        imprimir_pcb(pcb);
        cambiar_estado_pcb(pcb, READY);

        // imprimir_cantidad_procesos_disponibles_en_memoria();
        agregar_pcb_a_cola(pcb, COLA_READY);
        /*
        pthread_mutex_lock(&(COLA_READY->mutex));

        list_add(COLA_READY->lista_pcbs, pcb);

        pthread_mutex_unlock(&(COLA_READY->mutex));
        */
        sem_post(&HAY_PCB_DESALOJADO);
        // estado_conexion = CONEXION_FINALIZADA;
      } break;
      case -1: {
        xlog(COLOR_CONEXION, "Un proceso cliente se desconectó (socket=%d)", SOCKET_CONEXION_DISPATCH);

        // TODO: se debería actualizar el NEW
        // TODO: Evaluar si es necesario aumentar el grado de multiprogramacion
        // liberar_espacio_en_memoria_para_proceso();

        // centinela para detener el loop del hilo asociado a la conexión entrante
        estado_conexion = CONEXION_FINALIZADA;
        break;
      }
      case OPERACION_EXIT: {
        xlog(COLOR_CONEXION, "Se recibió solicitud para finalizar ejecución");

        log_destroy(logger), close(SOCKET_CONEXION_DISPATCH);
        // TODO: no estaría funcionando del todo, queda bloqueado en esperar_cliente()
        // Antes se esperaba un cliente pero ya no xq ya esxiste la conexion dispatch con kernel como cliente
        // ESTADO_CONEXION_DISPATCH = CONEXION_FINALIZADA;
        estado_conexion = CONEXION_FINALIZADA;

        // sem_post(&CERRAR_PROCESO);
      } break;
      default: {
        xlog(COLOR_ERROR, "Operacion %d desconocida", codigo_operacion);
      } break;
    }
  }

  // xlog(COLOR_CONEXION, "Se dejó de escuchar una (socket=%d, conexion=CPU_DISPATCH)", SOCKET_CONEXION_DISPATCH);
  pthread_exit(NULL);
}

void iniciar_conexion_cpu_dispatch() {
  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexion_cpu_dispatch, NULL);
  pthread_detach(th);
}

void iniciar_planificacion() {
  pthread_t th1, th2, th3, th4, th5;

  inicializar_grado_multiprogramacion();
  // sem_init(&CONEXION_DISPATCH_DISPONIBLE, 0, 0);
  sem_init(&HAY_PCB_DESALOJADO, 0, 0);
  sem_init(&NO_HAY_PROCESOS_EN_SUSREADY, 0, 1);
  sem_init(&MUTEX_BLOQUEO_SUSPENSION, 0, 1);
  COLA_NEW = cola_planificacion_create();
  COLA_READY = cola_planificacion_create();
  COLA_BLOCKED = cola_planificacion_create();
  COLA_SUSREADY = cola_planificacion_create();
  COLA_FINISHED = cola_planificacion_create();

  pthread_create(&th1, NULL, iniciar_largo_plazo, NULL), pthread_detach(th1);
  pthread_create(&th2, NULL, iniciar_corto_plazo, NULL), pthread_detach(th2);

  // Se mantiene la conexion dispatch, especialmente porque se deben escuchar por mensajes de esta conexion ademas de
  // enviar
  iniciar_conexion_cpu_dispatch();
  pthread_create(&th3, NULL, gestor_de_procesos_bloqueados, NULL), pthread_detach(th3);
  pthread_create(&th4, NULL, iniciar_mediano_plazo, NULL), pthread_detach(th4);

  pthread_create(&th5, NULL, (void *)escuchar_conexion_con_memoria, NULL), pthread_detach(th5);

  // sleep(1);

  // TODO: validar cuando debemos liberar los recursos asignados a las colas de planificación
  // cola_destroy(COLA_NEW);
  // cola_destroy(COLA_READY);
}

// TODO: validar agregando PCBs con instrucciones
void cola_destroy(t_cola_planificacion *cola) {
  list_destroy_and_destroy_elements(cola->lista_pcbs, (void *)pcb_destroy);
  free(cola);
}

// TODO: añadir un logger sólo para este planificador
void *iniciar_corto_plazo() {
  xlog(COLOR_INFO, "Planificador de Corto Plazo: Ejecutando...");

  sem_init(&EJECUTAR_ALGORITMO_PCP, 0, 0);

  while (1) {
    // Semaforo creado xq cuando se bloquea un proceso se debe mandar un nuevo proceso a cpu
    sem_wait(&EJECUTAR_ALGORITMO_PCP);

    xlog(COLOR_INFO, "PCP: Realizar toma de decision");
    sem_wait(&(COLA_READY->cantidad_procesos)); // Si no hay pcbs en ready se queda bloqueado aca hasta q haya
    // Ver transicion_new_a_ready para ver como se evita q el planificador siga si el algoritmo es FIFO y hay proceso
    // en ejecucion usando el semaforo EJECUTAR_ALGORITMO_PCP

    t_pcb *pcb_elegido_a_ejecutar = NULL;

    imprimir_proceso_en_running();
    if (!algoritmo_cargado_es("FIFO") && !algoritmo_cargado_es("SJF")) {
      xlog(COLOR_ERROR, "No hay un algoritmo de planificación cargado ó dicho algoritmo no está implementado");
    } else {
      if (algoritmo_cargado_es("SJF") && hay_algun_proceso_ejecutando()) {
        enviar_interrupcion();
        sem_wait(&HAY_PCB_DESALOJADO); // Se bloquea hasta recibir el pcb de cpu
        SE_ENVIO_INTERRUPCION = 0;
      }
    }

    pcb_elegido_a_ejecutar = elegir_pcb_segun_algoritmo(COLA_READY);
    imprimir_pcb(pcb_elegido_a_ejecutar);
    xlog(COLOR_TAREA,
         "Se seleccionó un Proceso para ejecutar en CPU (pid=%d, algoritmo=%s)",
         pcb_elegido_a_ejecutar->pid,
         obtener_algoritmo_cargado());

    ejecutar_proceso(pcb_elegido_a_ejecutar);

    SE_INDICO_A_PCP_QUE_REPLANIFIQUE = 0;
  }

  pthread_exit(NULL);
}

void ejecutar_proceso(t_pcb *pcb) {
  transicion_ready_a_running(pcb);
  xlog(COLOR_TAREA, "Cantidad de procesos en cola READY: %d", list_size(COLA_READY->lista_pcbs));

  t_paquete *paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  enviar_pcb(SOCKET_CONEXION_DISPATCH, paquete);
  // pcb_destroy(pcb); // Luego sera recibido uno igual pero actualizado
  // imprimir_pcb(pcb);
  // BEGIN = time(NULL);
  xlog(COLOR_INFO, "[TIMER]: Contando..");
  clock_gettime(CLOCK_REALTIME, &BEGIN);
  // timer_iniciar();
  // TODO: validar en el foro si se permite el escuchar la conexión dispatch desde kernel,
  // caso contrario deberiamos optar por algo asi
  // TODO: esto genera problemas para el envío/recepción de los paquetes apesar que esté sincronizado con semáforos

  // int socket_destino = conectarse_a_cpu("PUERTO_CPU_DISPATCH");
  /* if (socket_destino != -1) { */
  /*   t_paquete *paquete = paquete_create(); */
  /*   paquete_add_pcb(paquete, pcb); */
  /*   enviar_pcb(socket_destino, paquete); */
  /* } */
  /* close(socket_destino); */
}

void *iniciar_largo_plazo() {
  xlog(COLOR_INFO, "Planificador de Largo Plazo: Ejecutando...");

  pthread_t th;
  pthread_create(&th, NULL, plp_pcb_finished, NULL);
  pthread_detach(th);

  while (1) {
    sem_wait(&(COLA_NEW->cantidad_procesos));
    sem_wait(&HAY_PROCESOS_ENTRANTES);

    // xlog(COLOR_BLANCO, "Nuevo proceso Consola ingresar (pcbs=%d)", queue_size(PCBS_PROCESOS_ENTRANTES));

    t_pcb *pcb = elegir_pcb_fifo(COLA_NEW);

    // Esta funcion se encarga de priorizar SUSREADY sobre NEW y maneja el grado de Multiprogramacion
    controlar_procesos_disponibles_en_memoria(1); // Llamado por PLP

    transicion_new_a_ready(pcb);
    imprimir_cantidad_procesos_disponibles_en_memoria();

    // TODO: enviar_solicitud_tabla_paginas(fd_memoria);

    // TODO: contemplar cuando el proceso finaliza, por momento habrán memory leaks
    // pcb_destroy(pcb);
  }

  pthread_exit(NULL);
}

void *plp_pcb_finished() {
  xlog(COLOR_INFO, "Planificador de Largo Plazo: Funcion transicion finished ejecutando...");

  while (1) {
    sem_wait(&(COLA_FINISHED->cantidad_procesos));

    // TODO: Informar a memoria que termina el proceso y esperar respuesta
    t_pcb *pcb = elegir_pcb_fifo(COLA_FINISHED);
    remover_pcb_de_cola(pcb, COLA_FINISHED);
    imprimir_pcb(pcb);
    matar_proceso(pcb->socket); // Se avisa a la consola de la finalizacion
    pcb_destroy(pcb);
    imprimir_pcb(pcb);
  }
}

// Se encarga de realizar la transacion de SUSREADY a READY
void *iniciar_mediano_plazo() {
  xlog(COLOR_INFO, "Planificador de Mediano Plazo: Ejecutando...");

  while (1) {
    sem_wait(&(COLA_SUSREADY->cantidad_procesos));
    // sleep(10);
    t_pcb *pcb = elegir_pcb_fifo(COLA_SUSREADY);

    // Esta funcion se encarga de priorizar SUSREADY sobre NEW y maneja el grado de Multiprogramacion
    controlar_procesos_disponibles_en_memoria(0); // Llamado por PMP
    transicion_susready_a_ready(pcb);
    imprimir_cantidad_procesos_disponibles_en_memoria();

    // TODO: contemplar cuando el proceso finaliza, por momento habrán memory leaks
    // pcb_destroy(pcb);
  }

  pthread_exit(NULL);
}

void pmp_suspender_proceso(t_pcb *pcb) {
  pcb->estado = SUSBLOCKED;
  // TODO: Informar a memoria de suspension
  xlog(COLOR_INFO, "Se suspendio un proceso (pid = %d)", pcb->pid);
  liberar_espacio_en_memoria_para_proceso();
}

void *gestor_de_procesos_bloqueados() {
  while (1) {
    sem_wait(&(COLA_BLOCKED->cantidad_procesos));

    t_pcb *pcb = elegir_pcb_fifo(COLA_BLOCKED);
    xlog(COLOR_INFO, "Se bloqueara el proceso con pid=%d por un tiempo de %d", pcb->pid, pcb->tiempo_de_bloqueado);
    bloquear_por_milisegundos(pcb->tiempo_de_bloqueado);
    xlog(COLOR_INFO, "Finalizo el bloqueo del pcb=%d", pcb->pid);
    // TODO: Avisar a memoria fin de bloqueo y esperar respuesta

    // TODO: Evaluar si se necesita un semaforo para evitar la condicion de carrera al enviar mensajes a memoria
    // y cambiar el estado del pcb
    sem_wait(&MUTEX_BLOQUEO_SUSPENSION);
    if (pcb->estado != BLOCKED) {
      xlog(COLOR_INFO, "Proceso suspendido-bloqueado pasa a SUSREADY (pid = %d)", pcb->pid);
      transicion_blocked_a_susready(pcb);
    } else {
      pcb->estado = READY;
      xlog(COLOR_INFO, "Proceso bloqueado pasa a READY (pid = %d)", pcb->pid);
      transicion_blocked_a_ready(pcb);
    }
    sem_post(&MUTEX_BLOQUEO_SUSPENSION);
  }
}

int pcb_get_posicion(t_pcb *pcb, t_list *lista) {
  for (int posicion = 0; posicion < list_size(lista); posicion++) {
    if (pcb == (t_pcb *)list_get(lista, posicion)) {
      return posicion;
    }
  }
  return -1;
}

void agregar_pcb_a_cola(t_pcb *pcb, t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));

  list_add(cola->lista_pcbs, pcb);
  sem_post(&(cola->cantidad_procesos));

  pthread_mutex_unlock(&(cola->mutex));
}

void remover_pcb_de_cola(t_pcb *pcb, t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));
  int posicion = pcb_get_posicion(pcb, cola->lista_pcbs);

  if (posicion != -1) {
    list_remove(cola->lista_pcbs, posicion);
    // TODO: Validad que antes de llamar a esta funcion se este haciendo el sem_wait de cantidad_procesos
    // sem_wait(&(cola->cantidad_procesos));
    // No es necesario ya que para hacer las transiciones (que es para lo que esto se usa) ya se tuvo que hacer un wait
  } else {
    log_error(logger, "No existe tal elemento en la cola");
  }

  pthread_mutex_unlock(&(cola->mutex));
}

void cambiar_estado_pcb(t_pcb *pcb, t_pcb_estado nuevoEstado) {
  pcb->estado = nuevoEstado;
}

void transicion_ready_a_running(t_pcb *pcb) {
  /* cambiar_estado_pcb(pcb, RUNNING); */
  /* remover_pcb_de_cola(pcb, COLA_READY); */
  /* PROCESO_EJECUTANDO = pcb; */
  t_paquete *paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  solicitar_inicializar_estructuras_en_memoria(SOCKET_CONEXION_MEMORIA, paquete);
}

void transicion_running_a_blocked(t_pcb *pcb) {
  cambiar_estado_pcb(pcb, BLOCKED);
  agregar_pcb_a_cola(pcb, COLA_BLOCKED);

  liberar_cpu();
  pthread_t th;
  pthread_create(&th, NULL, (void *)timer_suspension_proceso, (void *)pcb), pthread_detach(th);
  xlog(COLOR_TAREA,
       "Transición de RUNNING a BLOCKED, el PCP atendió una operación de I/O (pid=%d, pcbs_en_blocked=%d, "
       "tiempo_bloqueo=%d)",
       pcb->pid,
       list_size(COLA_BLOCKED->lista_pcbs),
       pcb->tiempo_de_bloqueado);
}

void transicion_running_a_finished(t_pcb *pcb) {
  cambiar_estado_pcb(pcb, FINISHED);
  agregar_pcb_a_cola(pcb, COLA_FINISHED);

  liberar_cpu();

  xlog(COLOR_TAREA,
       "Transición de RUNNING a FINISHED, el PCP atendió una operación de FINISHED (pid=%d, pcbs_en_finished=%d)",
       pcb->pid,
       list_size(COLA_FINISHED->lista_pcbs));
}

void transicion_a_new(t_pcb *pcb) {
  cambiar_estado_pcb(pcb, NEW);
  agregar_pcb_a_cola(pcb, COLA_NEW);

  xlog(COLOR_TAREA,
       "Se agregó un PCB (pid=%d) a la cola de NEW (cantidad_pcbs=%d)",
       pcb->pid,
       list_size(COLA_NEW->lista_pcbs));
}

void transicion_new_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_NEW);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);

  xlog(COLOR_TAREA,
       "Transición de NEW a READY, el PLP aceptó un proceso en el Sistema (pid=%d, pcbs_en_new=%d, pcbs_en_ready=%d)",
       pcb->pid,
       list_size(COLA_NEW->lista_pcbs),
       list_size(COLA_READY->lista_pcbs));
  if (!SE_INDICO_A_PCP_QUE_REPLANIFIQUE) {
    xlog(COLOR_INFO, "No se habia indicado a pcp que replanifique");
    if (!algoritmo_cargado_es("FIFO") || !hay_algun_proceso_ejecutando()) {
      // !(algoritmo_cargado_es("FIFO") && hay_algun_proceso_ejecutando())
      sem_post(&EJECUTAR_ALGORITMO_PCP);
    }
  }
}

// TODO: Añadir un signal
void transicion_blocked_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_BLOCKED);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);
  if (!SE_INDICO_A_PCP_QUE_REPLANIFIQUE) {
    xlog(COLOR_INFO, "No se habia indicado a pcp que replanifique");
    if (!algoritmo_cargado_es("FIFO") || !hay_algun_proceso_ejecutando()) {
      // !(algoritmo_cargado_es("FIFO") && hay_algun_proceso_ejecutando())
      sem_post(&EJECUTAR_ALGORITMO_PCP);
    }
  }
}

// TODO: Añadir un signal
void transicion_blocked_a_susready(t_pcb *pcb) {
  t_paquete *paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);

  int socket_memoria = conectarse_a_memoria();
  solicitar_suspension_de_proceso(socket_memoria, paquete);

  remover_pcb_de_cola(pcb, COLA_BLOCKED);
  cambiar_estado_pcb(pcb, SUSREADY);
  agregar_pcb_a_cola(pcb, COLA_SUSREADY);

  if (list_size(COLA_SUSREADY->lista_pcbs) == 1) {
    sem_wait(&NO_HAY_PROCESOS_EN_SUSREADY);
  }

  xlog(COLOR_TAREA,
       "Se agregó un PCB (pid=%d) a la cola de SUSREADY (cantidad_pcbs=%d)",
       pcb->pid,
       list_size(COLA_SUSREADY->lista_pcbs));

  /*
  sem_post(&(COLA_SUSREADY->instancias_disponibles));
   */
}

/*
// TODO: Añadir un signal
void transicion_susblocked_a_susready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_SUSBLOCKED);
  cambiar_estado_pcb(pcb, SUSREADY);
  agregar_pcb_a_cola(pcb, COLA_SUSREADY);

  log_info(logger,
           "Se agregó un PCB (pid=%d) a la cola de SUSREADY (cantidad_pcbs=%d)",
           pcb->pid,
           list_size(COLA_SUSREADY->lista_pcbs));
}
*/

void transicion_susready_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_SUSREADY);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);

  if (list_size(COLA_SUSREADY->lista_pcbs) == 0) {
    sem_post(&NO_HAY_PROCESOS_EN_SUSREADY);
  }

  xlog(COLOR_TAREA,
       "Se agregó un PCB (pid=%d) de la cola de SUSREADY a la cola de READY (cantidad_pcbs=%d)",
       pcb->pid,
       list_size(COLA_READY->lista_pcbs));
}

t_cola_planificacion *cola_planificacion_create() {
  int sem_init_valor = 0;
  t_cola_planificacion *cola = malloc(sizeof(t_cola_planificacion));

  cola->lista_pcbs = list_create();
  pthread_mutex_init(&(cola->mutex), NULL);

  // TODO: evaluar si recibirlo por parámetro o definir una nueva función cola_asignar_instancias(cola, cantidad)
  sem_init(&(cola->cantidad_procesos), 0, sem_init_valor);

  xlog(COLOR_INFO, "Se creó una cola de planificación");

  return cola;
}

void inicializar_grado_multiprogramacion() {
  int grado = atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));
  sem_init(&PROCESOS_DISPONIBLES_EN_MEMORIA, 0, grado);
}

int obtener_cantidad_procesos_disponibles_en_memoria() {
  int grado;
  sem_getvalue(&PROCESOS_DISPONIBLES_EN_MEMORIA, &grado);
  return grado;
}

void imprimir_cantidad_procesos_disponibles_en_memoria() {
  xlog(COLOR_TAREA,
       "La cantidad de instancias de procesos disponibles para cargar en memoria actualmente es %d",
       obtener_cantidad_procesos_disponibles_en_memoria());
}

void liberar_espacio_en_memoria_para_proceso() {
  sem_post(&PROCESOS_DISPONIBLES_EN_MEMORIA);

  xlog(COLOR_TAREA, "Se libero un espacio en memoria para un nuevo proceso");
  imprimir_cantidad_procesos_disponibles_en_memoria();
}

void controlar_procesos_disponibles_en_memoria(int llamado_por_plp) {
  imprimir_cantidad_procesos_disponibles_en_memoria();
  xlog(COLOR_TAREA, "Controlamos contra el grado de multiprogramación antes de ingresar procesos al sistema");

  sem_wait(&PROCESOS_DISPONIBLES_EN_MEMORIA);
  while (llamado_por_plp && list_size(COLA_SUSREADY->lista_pcbs) != 0) {
    xlog(COLOR_TAREA, "Se entro en el ciclo del while al controlar los procesos disponibles en memoria");
    sem_post(&PROCESOS_DISPONIBLES_EN_MEMORIA);
    sem_wait(&NO_HAY_PROCESOS_EN_SUSREADY); // Usado para evitar espera activa
    sem_post(&NO_HAY_PROCESOS_EN_SUSREADY); // Usado para evitar posible deadlock si se entra en el ciclo otra vez
    sem_wait(&PROCESOS_DISPONIBLES_EN_MEMORIA);
  }

  imprimir_cantidad_procesos_disponibles_en_memoria();
}

t_pcb *elegir_pcb_fifo(t_cola_planificacion *cola) {
  t_pcb *pcb = NULL;

  pthread_mutex_lock(&(cola->mutex));
  pcb = (t_pcb *)list_get(cola->lista_pcbs, 0);
  pthread_mutex_unlock(&(cola->mutex));

  return pcb;
}

t_pcb *elegir_pcb_sjf(t_cola_planificacion *cola) {
  t_pcb *pcb = NULL;

  pthread_mutex_lock(&(cola->mutex));
  pcb = (t_pcb *)list_get_minimum(
    cola->lista_pcbs, (void *)pcb_menor_tiempo_restante_de_ejecucion_entre); // si hay empate devuelve por FIFO
  pthread_mutex_unlock(&(cola->mutex));

  return pcb;
}

t_pcb *pcb_menor_tiempo_restante_de_ejecucion_entre(t_pcb *pcb1, t_pcb *pcb2) {
  return pcb_tiempo_restante_de_ejecucion(pcb1) <= pcb_tiempo_restante_de_ejecucion(pcb2) ? pcb1 : pcb2;
}

int pcb_tiempo_restante_de_ejecucion(t_pcb *pcb) {
  return pcb->estimacion_rafaga - pcb->tiempo_en_ejecucion;
}

t_pcb *elegir_pcb_segun_algoritmo(t_cola_planificacion *cola) {
  t_pcb *pcb = NULL;

  if (algoritmo_cargado_es("FIFO")) {
    pcb = elegir_pcb_fifo(cola);
  }

  else if (algoritmo_cargado_es("SJF")) {
    pcb = elegir_pcb_sjf(cola);
  } else {
    xlog(COLOR_ERROR, "El algoritmo elegido no está cargado en el archivo de configuración");
  }

  return pcb;
}

char *obtener_algoritmo_cargado() {
  return config_get_string_value(config, "ALGORITMO_PLANIFICACION");
}

bool algoritmo_cargado_es(char *algoritmo) {
  return strcmp(obtener_algoritmo_cargado(), algoritmo) == 0;
}

void enviar_interrupcion() {
  t_paquete *paquete = paquete_create();
  paquete->codigo_operacion = OPERACION_INTERRUPT;

  // sem_wait(&CONEXION_DISPATCH_DISPONIBLE); // para sincronizar la respuesta de cpu con el pcb desalojado
  int socket_destino = conectarse_a_cpu("PUERTO_CPU_INTERRUPT");

  if (socket_destino != -1) {
    int status = enviar(socket_destino, paquete);

    if (status != -1) {
      xlog(COLOR_CONEXION, "La interrupción fue enviada con éxito (socket_destino=%d)", socket_destino);
      SE_ENVIO_INTERRUPCION = 1;
      close(socket_destino);
    }
  }
}

bool hay_algun_proceso_ejecutando() {
  return PROCESO_EJECUTANDO != NULL;
}

void liberar_cpu() {
  pcb_destroy(PROCESO_EJECUTANDO); // Genera core dumped
  PROCESO_EJECUTANDO = NULL;
}

void imprimir_proceso_en_running() {
  if (hay_algun_proceso_ejecutando()) {
    xlog(COLOR_INFO, "Hay algún proceso en running? SI (pid=%d)", PROCESO_EJECUTANDO->pid);
  } else {
    xlog(COLOR_INFO, "Hay algún proceso en running? NO");
  }
}

int calcular_estimacion_rafaga(t_pcb *pcb) {
  double alfa = config_get_double_value(config, "ALFA");
  xlog(COLOR_INFO, "El alfa es: %.2f", alfa);
  int estimacion_proxima_rafaga = alfa * pcb->tiempo_en_ejecucion + (1 - alfa) * pcb->estimacion_rafaga;
  return estimacion_proxima_rafaga;
}

int obtener_tiempo_maximo_bloqueado() {
  int tiempo_maximo_bloqueado = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");
  return tiempo_maximo_bloqueado;
}

void timer_suspension_proceso(t_pcb *pcb) {
  xlog(COLOR_INFO, "Comenzando timer de suspension (pid = %d)...", pcb->pid);
  pcb_timer_t timer_suspension;
  int tiempo_maximo_bloqueado = obtener_tiempo_maximo_bloqueado();
  timer_suspension.timer_inicio = clock();

  do {
    timer_suspension.timer_fin = clock();

    timer_suspension.tiempo_total = (timer_suspension.timer_fin - timer_suspension.timer_inicio) / 1000;
  } while (pcb->estado == BLOCKED && timer_suspension.tiempo_total < tiempo_maximo_bloqueado);

  xlog(COLOR_INFO, "Finalizando timer de suspension (pid = %d)", pcb->pid);
  // TODO: Evaluar si se necesita un semaforo para evitar la condicion de carrera al enviar mensajes a memoria
  // y cambiar el estado del pcb
  sem_wait(&MUTEX_BLOQUEO_SUSPENSION);
  if (pcb->estado == BLOCKED) {
    pmp_suspender_proceso(pcb);
  }
  sem_post(&MUTEX_BLOQUEO_SUSPENSION);

  pthread_exit(NULL);
}

int conectarse_a_memoria() {
  char *ip = config_get_string_value(config, "IP_MEMORIA");
  char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
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
        xlog(COLOR_CONEXION, "Se recibió confirmación de Memoria para suspender proceso");

        // TODO: sincronizar con semáforos donde corresponda
      } break;
      case OPERACION_ESTRUCTURAS_EN_MEMORIA_CONFIRMADO: {
        xlog(COLOR_CONEXION, "Se recibió confirmación de Memoria estructuras inicializadas para un proceso");

        // TODO: sincronizar con semáforos donde corresponda
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
