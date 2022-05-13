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
// TODO: Evaluar si esta variable global es necesaria
// CONEXION_ESTADO ESTADO_CONEXION_DISPATCH;

sem_t CONEXION_DISPATCH_DISPONIBLE; // semáforo binario
sem_t HAY_PCB_DESALOJADO;           // semáforo binario
sem_t EJECUTAR_ALGORITMO_PCP;       // semaforo binario
time_t BEGIN;
time_t END;

void avisar_a_pcp_que_decida() {
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
    END = time(NULL);
    timer_detener();
    timer_imprimir();
    /*
    struct tm *ptr;
    time_t t;
    t = time(NULL);
    ptr = localtime(&t);
    printf("%s", asctime(ptr));
    */
    xlog(COLOR_INFO, "Tiempo que pcb estuvo en cpu: %lf", difftime(END, BEGIN));

    switch (codigo_operacion) {
      case OPERACION_PCB_CON_IO: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_DISPATCH);
        t_pcb *pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        xlog(COLOR_PAQUETE, "Se recibió un pcb con operación de I/O (pid=%d)", pcb->pid);
        xlog(COLOR_INFO, "Se bloquea un proceso (pid=%d, tiempo=%d)", pcb->pid, pcb->tiempo_de_bloqueado);

        pcb->tiempo_en_ejecucion += TIMER.tiempo_total; // en milisegundos

        pcb->estimacion_rafaga = calcular_estimacion_rafaga(pcb);
        pcb->tiempo_en_ejecucion = 0;


        // TODO: se debe bloquear el proceso, sin bloquear la escucha
        transicion_running_a_blocked(pcb);
        avisar_a_pcp_que_decida(); // Le indico al pcb q debe realizar una eleccion ya q cpu esta vacia
        imprimir_pcb(pcb);
      } break;
      case OPERACION_PCB_CON_EXIT: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_DISPATCH);
        t_pcb *pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        // TODO: sincronizar con plp para matar el proceso, y hacer transicion de estado/cola

        xlog(COLOR_PAQUETE, "Se recibió un pcb con operación EXIT (pid=%d)", pcb->pid);
        xlog(COLOR_INFO, "Se finaliza un proceso (pid=%d)", pcb->pid);
      } break;
      case OPERACION_PCB_DESALOJADO: {
        t_paquete *paquete = recibir_paquete(SOCKET_CONEXION_DISPATCH);
        t_pcb *pcb = paquete_obtener_pcb(paquete);
        paquete_destroy(paquete);

        pcb->tiempo_en_ejecucion += TIMER.tiempo_total; // en milisegundos

        liberar_cpu();
        xlog(COLOR_PAQUETE, "Se recibió un pcb desalojado (pid=%d)", pcb->pid);

        imprimir_pcb(pcb);
        cambiar_estado_pcb(pcb, READY);

        // imprimir_grado_multiprogramacion_actual();
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
        // bajar_grado_multiprogramacion();
        actualizar_grado_multiprogramacion();

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
      default: { xlog(COLOR_ERROR, "Operacion %d desconocida", codigo_operacion); } break;
    }
  }

  // xlog(COLOR_CONEXION, "Se dejó de escuchar una (socket=%d, conexion=CPU_DISPATCH)", SOCKET_CONEXION_DISPATCH);
  pthread_exit(NULL);
}

void iniciar_conexion_cpu_dispatch() {
  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexion_cpu_dispatch, NULL), pthread_detach(th);
}

void iniciar_planificacion() {
  pthread_t th1, th2, th3;

  inicializar_grado_multiprogramacion();
  sem_init(&CONEXION_DISPATCH_DISPONIBLE, 0, 0);
  sem_init(&HAY_PCB_DESALOJADO, 0, 0);
  COLA_NEW = cola_planificacion_create();
  COLA_READY = cola_planificacion_create();
  COLA_BLOCKED = cola_planificacion_create();

  // TODO: descomentar a medida que se vayan implementando los planificadores
  /* COLA_BLOCKED = inicializar_cola(COLA_BLOCKED); */
  COLA_SUSREADY = cola_planificacion_create();
  COLA_SUSBLOCKED = cola_planificacion_create();

  pthread_create(&th1, NULL, iniciar_largo_plazo, NULL), pthread_detach(th1);
  pthread_create(&th2, NULL, iniciar_corto_plazo, NULL), pthread_detach(th2);

  pthread_create(&th3, NULL, escuchar_conexion_cpu_dispatch, NULL), pthread_detach(th3);

  // pthread_create(&th3, NULL, iniciar_mediano_plazo, NULL), pthread_detach(th3);
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
  // TODO: evaluar si corresponde conectar/desconectar a cada rato, ó si solo mantenemos la conexión
  // Decidi mantener la conexion, especialmente porque se deben escuchar por mensajes de esta conexion ademas de enviar
  // pthread_t th;
  // pthread_create(&th, NULL, iniciar_conexion_cpu_dispatch, NULL), pthread_detach(th);

  while (1) {
    sem_wait(&EJECUTAR_ALGORITMO_PCP); // Semaforo creado xq cuando se bloquea un proceso se debe mandar un nuevo
                                       // proceso a cpu
    xlog(COLOR_INFO, "PCP: Realizar toma de decision");
    sem_wait(&(COLA_READY->cantidad_procesos)); // pero si no hay pcbs en ready se queda bloqueado aca hasta q haya
    // Ver transicion_new_a_ready para ver como se evita q el planificador siga si el algoritmo es FIFO y hay proceso
    // en ejecucion usando el semaforo EJECUTAR_ALGORITMO_PCP

    // TODO: Si el semaforo EJECUTAR_ALGORITMO_PCP tiene valor 1, disminuirlo

    t_pcb *pcb_elegido_a_ejecutar = NULL;

    imprimir_proceso_en_running();
    if (!algoritmo_cargado_es("FIFO") && !algoritmo_cargado_es("SJF")) {
      xlog(COLOR_ERROR, "No hay un algoritmo de planificación cargado ó dicho algoritmo no está implementado");
    } else {
      if (algoritmo_cargado_es("SJF") && hay_algun_proceso_ejecutando()) {
        // TODO: validar en el foro, ya que si no se está realizando un handshake y no lo solicitan

        enviar_interrupcion();
        // iniciar_conexion_cpu_dispatch(), enviar_interrupcion();
        sem_wait(&HAY_PCB_DESALOJADO); // se bloquea hasta recibir el pcb de cpu
      }
    }

    pcb_elegido_a_ejecutar = elegir_pcb_segun_algoritmo(COLA_READY);
    imprimir_pcb(pcb_elegido_a_ejecutar);
    xlog(COLOR_TAREA,
         "Se seleccionó un Proceso para ejecutar en CPU (pid=%d, algoritmo=%s)",
         pcb_elegido_a_ejecutar->pid,
         obtener_algoritmo_cargado());

    ejecutar_proceso(pcb_elegido_a_ejecutar);
  }

  pthread_exit(NULL);
}

void ejecutar_proceso(t_pcb *pcb) {
  transicion_ready_a_running(pcb);
  xlog(COLOR_TAREA, "Cantidad de procesos en cola READY: %d", list_size(COLA_READY->lista_pcbs));

  t_paquete *paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  enviar_pcb(SOCKET_CONEXION_DISPATCH, paquete);
  BEGIN = time(NULL);
  timer_iniciar();
  /*
  struct tm *ptr;
  time_t t;
  t = time(NULL);
  ptr = localtime(&t);
  printf("%s", asctime(ptr));
  */
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

// TODO: se deben cambiar de estado a EXIT y remover de la cola de READY...
// cuando se desconecten ó cuando terminen sus hilos
void *iniciar_largo_plazo() {
  xlog(COLOR_INFO, "Planificador de Largo Plazo: Ejecutando...");

  while (1) {
    sem_wait(&(COLA_NEW->cantidad_procesos));
    sem_wait(&HAY_PROCESOS_ENTRANTES);

    // xlog(COLOR_BLANCO, "Nuevo proceso Consola ingresar (pcbs=%d)", queue_size(PCBS_PROCESOS_ENTRANTES));

    t_pcb *pcb = (t_pcb *)queue_pop(PCBS_PROCESOS_ENTRANTES);
    transicion_a_new(pcb);

    // TODO: creo que no tengo que hacer bajar_grado...
    // TODO: me parece que debo usar un wait_condition que compare el valor de  READY contra el grado de
    // multiprogramacion?
    // TODO: después tendrias que chequear SUSREADY+READY

    // controlar_grado_multiprogramacion();
    transicion_new_a_ready(pcb), imprimir_grado_multiprogramacion_actual();

    // TODO: enviar_solicitud_tabla_paginas(fd_memoria);

    // TODO: contemplar cuando el proceso finaliza, por momento habrán memory leaks
    // pcb_destroy(pcb);
  }

  pthread_exit(NULL);
}

// TODO: definir
void *iniciar_mediano_plazo() {
  xlog(COLOR_INFO, "Planificador de Mediano Plazo: Ejecutando...");

  // TODO: Agregar logica transicion de blocked a susblocked

  while (1) {
    // sem_wait(&(COLA_SUSREADY->instancias_disponibles));

    t_pcb *pcb = elegir_pcb_fifo(COLA_SUSREADY);

    // controlar_grado_multiprogramacion();
    transicion_susready_a_ready(pcb);

    // TODO: contemplar cuando el proceso finaliza, por momento habrán memory leaks
    // pcb_destroy(pcb);
  }

  pthread_exit(NULL);
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
  sem_post(&(cola->cantidad_procesos)); // sem++

  pthread_mutex_unlock(&(cola->mutex));
}

void remover_pcb_de_cola(t_pcb *pcb, t_cola_planificacion *cola) {
  pthread_mutex_lock(&(cola->mutex));
  int posicion = pcb_get_posicion(pcb, cola->lista_pcbs);

  if (posicion != -1) {
    list_remove(cola->lista_pcbs, posicion);
  } else {
    log_error(logger, "No existe tal elemento en la cola");
  }

  // sem_wait(&(cola->cantidad_procesos)); // sem--
  pthread_mutex_unlock(&(cola->mutex));
}

void cambiar_estado_pcb(t_pcb *pcb, t_pcb_estado nuevoEstado) {
  pcb->estado = nuevoEstado;
}

void transicion_ready_a_running(t_pcb *pcb) {
  cambiar_estado_pcb(pcb, RUNNING);
  remover_pcb_de_cola(pcb, COLA_READY);

  PROCESO_EJECUTANDO = pcb;
}

void transicion_running_a_blocked(t_pcb *pcb) {
  cambiar_estado_pcb(pcb, BLOCKED);
  agregar_pcb_a_cola(pcb, COLA_BLOCKED);

  liberar_cpu();

  xlog(COLOR_TAREA,
       "Transición de RUNNING a BLOCKED, el PCP atendió una operación de I/O (pid=%d, pcbs_en_blocked=%d, "
       "tiempo_bloqueo=%d)",
       pcb->pid,
       list_size(COLA_BLOCKED->lista_pcbs),
       pcb->tiempo_de_bloqueado);
}

void transicion_a_new(t_pcb *pcb) {
  agregar_pcb_a_cola(pcb, COLA_NEW);

  // sem_post(&(COLA_NEW->instancias_disponibles));
  sem_post(&(COLA_NEW->cantidad_procesos));

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

  if (!algoritmo_cargado_es("FIFO") ||
      !hay_algun_proceso_ejecutando()) { // !(algoritmo_cargado_es("FIFO") && hay_algun_proceso_ejecutando())
    sem_post(&EJECUTAR_ALGORITMO_PCP);
  }

  /*
   *
  liberar_una_instancia_de_recurso(COLA_NEW); // sem++
  tomar_una_instancia_de_recurso(COLA_READY); // sem--

  sem_wait(&(COLA_NEW->instancias_disponibles));
  sem_post(&(COLA_READY->instancias_disponibles));
   */
}

// TODO: Añadir un signal
void transicion_blocked_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_BLOCKED);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);
}

// TODO: Añadir un signal
void transicion_susblocked_a_susready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_SUSBLOCKED);
  cambiar_estado_pcb(pcb, SUSREADY);
  agregar_pcb_a_cola(pcb, COLA_SUSREADY);

  log_info(logger,
           "Se agregó un PCB (pid=%d) a la cola de SUSREADY (cantidad_pcbs=%d)",
           pcb->pid,
           list_size(COLA_SUSREADY->lista_pcbs));

  /*
  if (list_size(COLA_SUSREADY->lista_pcbs) == 1) {
    pthread_mutex_lock(&NO_HAY_PROCESOS_EN_SUSREADY);
  }

  sem_post(&(COLA_SUSREADY->instancias_disponibles));
   */
}

void transicion_susready_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_SUSREADY);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);

  log_info(logger,
           "Se agregó un PCB (pid=%d) a la cola de READY (cantidad_pcbs=%d)",
           pcb->pid,
           list_size(COLA_READY->lista_pcbs));
  /*
  sem_post(&(COLA_READY->instancias_disponibles));

  if (list_size(COLA_SUSREADY->lista_pcbs) == 0) {
    pthread_mutex_unlock(&NO_HAY_PROCESOS_EN_SUSREADY);
  }
   */
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
  sem_init(&GRADO_MULTIPROGRAMACION, 0, grado);
}

int obtener_grado_multiprogramacion_actual() {
  int grado;
  // sem_getvalue(&GRADO_MULTIPROGRAMACION, &grado);

  // TODO: se debe contemplar también el susready?
  pthread_mutex_lock(&(COLA_READY->mutex));
  // grado = list_size(COLA_READY->lista_pcbs);
  sem_getvalue(&(COLA_READY->cantidad_procesos), &grado);
  pthread_mutex_unlock(&(COLA_READY->mutex));

  return grado;
}

void imprimir_grado_multiprogramacion_actual() {
  xlog(COLOR_TAREA, "El grado de multiprogramación actual es %d", obtener_grado_multiprogramacion_actual());
}
/*
int obtener_grado_multiprogramacion_por_config() {
  int grado = atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));

  return grado;
}
*/

// TODO: evaluar si corresponde manejar esto, el valor es fijo y no deberíamos modificarlo
void subir_grado_multiprogramacion() {
  // sem_post(&GRADO_MULTIPROGRAMACION); // hace sem++
  // xlog(COLOR_AMARILLO, "Subió el grado de multiprogramación (grado=%d)", obtener_grado_multiprogramacion());
}

// TODO: evaluar si corresponde manejar esto, el valor es fijo y no deberíamos modificarlo
void bajar_grado_multiprogramacion() {
  // sem_wait(&GRADO_MULTIPROGRAMACION);
  // xlog(COLOR_AMARILLO, "Bajó el grado de multiprogramación (grado=%d)", obtener_grado_multiprogramacion());
}

void actualizar_grado_multiprogramacion() {
  sem_post(&GRADO_MULTIPROGRAMACION);

  xlog(COLOR_TAREA, "Se actualizó el grado de multiprogramación");
  imprimir_grado_multiprogramacion_actual();
}

void controlar_grado_multiprogramacion() {
  sem_wait(&GRADO_MULTIPROGRAMACION);

  xlog(COLOR_TAREA, "Controlamos el grado de multiprogramación antes de ingresar procesos al sistema");
  imprimir_grado_multiprogramacion_actual();
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

      close(socket_destino);
    }
  }
}

bool hay_algun_proceso_ejecutando() {
  return PROCESO_EJECUTANDO != NULL;
}

void liberar_cpu() {
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