#include "planificador.h"

int SE_ENVIO_INTERRUPCION = 0;
t_pcb *PROCESO_EJECUTANDO = NULL;

// Para que si alguien le pidio al PCP que replanifique
// y este no lo hizo (por ejemplo porque no hay procesos en ready)
// no se vuelva a pedir y se siga aumentando el valor de los semaforos
int SE_INDICO_A_PCP_QUE_REPLANIFIQUE = 0;

void iniciar_planificacion() {
  pthread_t th1, th2, th3, th4, th5;

  inicializar_grado_multiprogramacion();
  sem_init(&HAY_PCB_DESALOJADO, 0, 0);
  sem_init(&SUSPENSION_EXITOSA, 0, 0);
  sem_init(&INICIALIZACION_ESTRUCTURAS_EXITOSA, 0, 0);
  sem_init(&LIBERACION_RECURSOS_EXITOSA, 0, 0);
  sem_init(&HAY_PCB_FINISH, 0, 0);
  sem_init(&NO_HAY_PROCESOS_EN_SUSREADY, 0, 1);
  sem_init(&MUTEX_BLOQUEO_SUSPENSION, 0, 1);
  COLA_NEW = cola_planificacion_create();
  COLA_READY = cola_planificacion_create();
  COLA_BLOCKED = cola_planificacion_create();
  COLA_SUSREADY = cola_planificacion_create();
  COLA_FINISHED = cola_planificacion_create();

  pthread_create(&th1, NULL, iniciar_largo_plazo, NULL), pthread_detach(th1);
  pthread_create(&th2, NULL, iniciar_corto_plazo, NULL), pthread_detach(th2);

  // Se mantiene la conexion dispatch, para escuchar/enviar mensajes
  iniciar_conexion_cpu_dispatch();
  pthread_create(&th3, NULL, gestor_de_procesos_bloqueados, NULL);
  pthread_detach(th3);

  pthread_create(&th4, NULL, iniciar_mediano_plazo, NULL);
  pthread_detach(th4);

  pthread_create(&th5, NULL, (void *)escuchar_conexion_con_memoria, NULL);
  pthread_detach(th5);
}

void cola_destroy(t_cola_planificacion *cola) {
  list_destroy_and_destroy_elements(cola->lista_pcbs, (void *)pcb_destroy);
  free(cola);
}

void ejecutar_proceso(t_pcb *pcb) {
  transicion_ready_a_running(pcb);
  xlog(COLOR_TAREA, "Cantidad de procesos en cola READY: %d", list_size(COLA_READY->lista_pcbs));

  t_paquete *paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);
  enviar_pcb(SOCKET_CONEXION_DISPATCH, paquete);
  paquete_destroy(paquete);
  xlog(COLOR_INFO, "[TIMER]: Contando..");
  clock_gettime(CLOCK_REALTIME, &BEGIN);
}

void *plp_pcb_finished() {
  xlog(COLOR_INFO, "Planificador de Largo Plazo: Funcion transicion finished ejecutando...");

  while (1) {
    sem_wait(&HAY_PCB_FINISH);
    sem_wait(&(COLA_FINISHED->cantidad_procesos));

    t_pcb *pcb = elegir_pcb_fifo(COLA_FINISHED);

    // Informar a memoria que termina el proceso
    t_paquete *paquete = paquete_create();
    paquete_add_pcb(paquete, pcb);
    solicitar_liberar_recursos_en_memoria_swap(SOCKET_CONEXION_MEMORIA, paquete);
    paquete_destroy(paquete);

    remover_pcb_de_cola(pcb, COLA_FINISHED);
    imprimir_pcb(pcb);
    matar_proceso(pcb->socket); // Se avisa a la consola de la finalizacion
    pcb_destroy(pcb);
  }
}

// No se necesita una transición a una cola de suspensión, con tener un cambio de estado es suficiente
// https://github.com/sisoputnfrba/foro/issues/2639
void pmp_suspender_proceso(t_pcb *pcb) {
  t_paquete *paquete = paquete_create();
  paquete_add_pcb(paquete, pcb);

  // Informo a memoria de suspension y espero respuesta de exito
  solicitar_suspension_de_proceso(SOCKET_CONEXION_MEMORIA, paquete);
  paquete_destroy(paquete);
  sem_wait(&SUSPENSION_EXITOSA);

  pcb->estado = SUSBLOCKED;
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

    // Semaforo para evitar condicion de carrera entre bloqueo y suspension
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
  } else {
    log_error(logger, "No existe tal elemento en la cola");
  }

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

  sem_post(&HAY_PCB_FINISH);
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
  evaluar_replanificacion_pcp();
}

void transicion_blocked_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_BLOCKED);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);
  evaluar_replanificacion_pcp();
}

void transicion_blocked_a_susready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_BLOCKED);
  cambiar_estado_pcb(pcb, SUSREADY);
  agregar_pcb_a_cola(pcb, COLA_SUSREADY);

  // nota temporal: si hay un proceso en SUSREADY => se bloquea el semáforo binario
  if (list_size(COLA_SUSREADY->lista_pcbs) == 1) {
    sem_wait(&NO_HAY_PROCESOS_EN_SUSREADY);
  }

  xlog(COLOR_TAREA,
       "Se agregó un PCB (pid=%d) a la cola de SUSREADY (cantidad_pcbs=%d)",
       pcb->pid,
       list_size(COLA_SUSREADY->lista_pcbs));
}

void transicion_susready_a_ready(t_pcb *pcb) {
  remover_pcb_de_cola(pcb, COLA_SUSREADY);
  cambiar_estado_pcb(pcb, READY);
  agregar_pcb_a_cola(pcb, COLA_READY);

  // nota temporal: si no hay procesos en SUSREADY => se desbloquea el semáforo binario
  if (list_size(COLA_SUSREADY->lista_pcbs) == 0) {
    sem_post(&NO_HAY_PROCESOS_EN_SUSREADY);
  }

  xlog(COLOR_TAREA,
       "Se agregó un PCB (pid=%d) de la cola de SUSREADY a la cola de READY (cantidad_pcbs=%d)",
       pcb->pid,
       list_size(COLA_READY->lista_pcbs));

  evaluar_replanificacion_pcp();
}

void evaluar_replanificacion_pcp() {
  if (!SE_INDICO_A_PCP_QUE_REPLANIFIQUE) {
    xlog(COLOR_INFO, "No se habia indicado a pcp que replanifique");

    if (!algoritmo_cargado_es("FIFO") || !hay_algun_proceso_ejecutando()) {
      // !(algoritmo_cargado_es("FIFO") && hay_algun_proceso_ejecutando()) => criterio original sin distribuir negacion
      avisar_a_pcp_que_decida();
    }
  }
}

void avisar_a_pcp_que_decida() {
  SE_INDICO_A_PCP_QUE_REPLANIFIQUE = 1;
  sem_post(&EJECUTAR_ALGORITMO_PCP);
}

t_cola_planificacion *cola_planificacion_create() {
  int sem_init_valor = 0;
  t_cola_planificacion *cola = malloc(sizeof(t_cola_planificacion));

  cola->lista_pcbs = list_create();
  pthread_mutex_init(&(cola->mutex), NULL);

  sem_init(&(cola->cantidad_procesos), 0, sem_init_valor);

  xlog(COLOR_INFO, "Se creó una cola de planificación");

  return cola;
}

void inicializar_grado_multiprogramacion() {
  int grado = obtener_grado_multiprogramacion_por_config();

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
  xlog(COLOR_TAREA, "Se libero un espacio en memoria para un nuevo proceso");

  sem_post(&PROCESOS_DISPONIBLES_EN_MEMORIA);
  // imprimir_cantidad_procesos_disponibles_en_memoria();
}

void controlar_procesos_disponibles_en_memoria(int llamado_por_plp) {
  imprimir_cantidad_procesos_disponibles_en_memoria();
  xlog(COLOR_TAREA, "Controlamos contra el grado de multiprogramación antes de ingresar procesos al sistema");

  sem_wait(&PROCESOS_DISPONIBLES_EN_MEMORIA);

  // Se entra si el PLP es quien pide pasar un proceso a READY segun el grado de multiprogramacion
  // pero hay procesos en SUSREADY
  while (llamado_por_plp && list_size(COLA_SUSREADY->lista_pcbs) != 0) {
    xlog(COLOR_TAREA, "Se entro en el ciclo del while al controlar los procesos disponibles en memoria");
    // Se le permite paso al wait en el que se encuentra o se valla a encontrar el PMP con un proceso en SUSREADY
    sem_post(&PROCESOS_DISPONIBLES_EN_MEMORIA);

    sem_wait(&NO_HAY_PROCESOS_EN_SUSREADY); // Usado para evitar espera activa
    sem_post(&NO_HAY_PROCESOS_EN_SUSREADY); // Usado para evitar posible deadlock si se entra en el ciclo otra vez

    // Aunque ya no haya procesos en SUSREADY se debe esperar a que se libere un espacio segun el grado de
    // multiprogramacion
    sem_wait(&PROCESOS_DISPONIBLES_EN_MEMORIA);

    // Luego de esto si otro proceso entra en SUSREADY se volvera a entrar en el ciclo y se le dara prioridad a ese
    // proceso
  }

  xlog(COLOR_TAREA, "Se acepto a un proceso en memoria");
  // imprimir_cantidad_procesos_disponibles_en_memoria();
}

t_pcb *elegir_pcb_fifo(t_cola_planificacion *cola) {
  t_pcb *pcb = NULL;

  pthread_mutex_lock(&(cola->mutex));
  pcb = (t_pcb *)list_get(cola->lista_pcbs, 0);
  pthread_mutex_unlock(&(cola->mutex));

  return pcb;
}

t_pcb *elegir_pcb_srt(t_cola_planificacion *cola) {
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
  return (int)pcb->estimacion_rafaga - (int)pcb->tiempo_en_ejecucion;
}

t_pcb *elegir_pcb_segun_algoritmo(t_cola_planificacion *cola) {
  t_pcb *pcb = NULL;

  if (algoritmo_cargado_es("FIFO")) {
    pcb = elegir_pcb_fifo(cola);
  }

  else if (algoritmo_cargado_es("SRT")) {
    pcb = elegir_pcb_srt(cola);
  } else {
    xlog(COLOR_ERROR, "El algoritmo elegido no está cargado en el archivo de configuración");
  }

  return pcb;
}

bool algoritmo_cargado_es(char *algoritmo) {
  return strcmp(obtener_algoritmo_cargado(), algoritmo) == 0;
}

bool hay_algun_proceso_ejecutando() {
  return PROCESO_EJECUTANDO != NULL;
}

void liberar_cpu() {
  pcb_destroy(PROCESO_EJECUTANDO);
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
  double alfa = obtener_alfa_por_config();

  xlog(COLOR_INFO, "El alfa es: %.2f", alfa);
  int estimacion_proxima_rafaga = alfa * pcb->tiempo_en_ejecucion + (1 - alfa) * pcb->estimacion_rafaga;
  return estimacion_proxima_rafaga;
}

void timer_suspension_proceso(t_pcb *pcb) {
  xlog(COLOR_INFO, "Comenzando timer de suspension (pid = %d)...", pcb->pid);
  struct timespec timer_suspension_inicio;
  struct timespec timer_suspension_fin;
  int tiempo_timer_suspension;
  int tiempo_maximo_bloqueado = obtener_tiempo_maximo_bloqueado();
  clock_gettime(CLOCK_REALTIME, &timer_suspension_inicio);

  do {
    clock_gettime(CLOCK_REALTIME, &timer_suspension_fin);

    tiempo_timer_suspension = (timer_suspension_fin.tv_sec - timer_suspension_inicio.tv_sec) * 1000 +
                              (timer_suspension_fin.tv_nsec - timer_suspension_inicio.tv_nsec) / 1000000;
  } while (pcb->estado == BLOCKED && tiempo_timer_suspension < tiempo_maximo_bloqueado);

  xlog(COLOR_INFO, "Finalizando timer de suspension (pid = %d)", pcb->pid);

  // Semaforo para evitar condicion de carrera entre bloqueo y suspension
  sem_wait(&MUTEX_BLOQUEO_SUSPENSION);
  if (pcb->estado == BLOCKED) {
    pmp_suspender_proceso(pcb);
  }
  sem_post(&MUTEX_BLOQUEO_SUSPENSION);

  pthread_exit(NULL);
}
