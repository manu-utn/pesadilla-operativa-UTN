#include "planificador.h"

void *iniciar_corto_plazo() {
  xlog(COLOR_INFO, "Planificador de Corto Plazo: Ejecutando...");

  // semaforo binario inicializado como deshabilitado
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
    if (!algoritmo_cargado_es("FIFO") && !algoritmo_cargado_es("SRT")) {
      xlog(COLOR_ERROR, "No hay un algoritmo de planificación cargado ó dicho algoritmo no está implementado");
    } else {
      if (algoritmo_cargado_es("SRT") && hay_algun_proceso_ejecutando()) {
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

// Se encarga de realizar la transacion de SUSREADY a READY
void *iniciar_mediano_plazo() {
  xlog(COLOR_INFO, "Planificador de Mediano Plazo: Ejecutando...");

  while (1) {
    sem_wait(&(COLA_SUSREADY->cantidad_procesos));

    t_pcb *pcb = elegir_pcb_fifo(COLA_SUSREADY);

    // Esta funcion se encarga de priorizar SUSREADY sobre NEW y maneja el grado de Multiprogramacion
    controlar_procesos_disponibles_en_memoria(0); // Llamado por PMP
    transicion_susready_a_ready(pcb);
    imprimir_cantidad_procesos_disponibles_en_memoria();
  }

  pthread_exit(NULL);
}

void *iniciar_largo_plazo() {
  xlog(COLOR_INFO, "Planificador de Largo Plazo: Ejecutando...");

  pthread_t th;
  pthread_create(&th, NULL, plp_pcb_finished, NULL);
  pthread_detach(th);

  while (1) {
    sem_wait(&(COLA_NEW->cantidad_procesos));
    sem_wait(&HAY_PROCESOS_ENTRANTES);

    t_pcb *pcb = elegir_pcb_fifo(COLA_NEW);

    // Esta funcion se encarga de priorizar SUSREADY sobre NEW y maneja el grado de Multiprogramacion
    controlar_procesos_disponibles_en_memoria(1); // Llamado por PLP

    t_paquete *paquete = paquete_create();
    paquete_add_pcb(paquete, pcb);
    solicitar_inicializar_estructuras_en_memoria(SOCKET_CONEXION_MEMORIA, paquete);
    paquete_destroy(paquete);

    sem_wait(&INICIALIZACION_ESTRUCTURAS_EXITOSA);
    pcb->tabla_primer_nivel = REFERENCIA_TABLA_RECIBIDA;

    transicion_new_a_ready(pcb);
    imprimir_cantidad_procesos_disponibles_en_memoria();
  }

  pthread_exit(NULL);
}
