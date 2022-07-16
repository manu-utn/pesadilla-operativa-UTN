#include "planificador.h"

void imprimir_cantidad_procesos_disponibles_en_memoria() {
  xlog(COLOR_TAREA, "La cantidad de instancias de procesos disponibles para cargar en memoria actualmente es %d", obtener_cantidad_procesos_disponibles_en_memoria());
}

void imprimir_proceso_en_running() {
  if (hay_algun_proceso_ejecutando()) {
    xlog(COLOR_INFO, "Hay algún proceso en running? SI (pid=%d)", PROCESO_EJECUTANDO->pid);
  } else {
    xlog(COLOR_INFO, "Hay algún proceso en running? NO");
  }
}
