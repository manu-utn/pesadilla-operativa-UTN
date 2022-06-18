#include "libstatic.h"
#include "memoria.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/collections/dictionary.h>
#include <commons/string.h>

int ejecutar_reemplazo(int pid, t_entrada_tabla_segundo_nivel* entrada) {
  t_list* marcos_asignados_al_proceso = obtener_marcos_asignados_a_este_proceso(pid);

  t_entrada_tabla_segundo_nivel* entrada_victima = malloc(sizeof(t_entrada_tabla_segundo_nivel));

  if(algoritmo_reemplazo_cargado_es("CLOCK")){
    entrada_victima = ejecutar_clock(marcos_asignados_al_proceso, entrada);

    // TODO: REALIZAR EL REEMPLAZO ENTRE LA ENTRADA A REEMPLAZAR Y LA ENTRADA REFERENCIADA

  } else {
    // marco_victima = ejecutar_clock_modificado(marcos_involucrados, entrada);
  }

  return 0;
}

t_entrada_tabla_segundo_nivel* ejecutar_clock(t_list* marcos, t_entrada_tabla_segundo_nivel* entrada) {
  t_entrada_tabla_segundo_nivel* entrada_victima = malloc(sizeof(t_entrada_tabla_segundo_nivel));

  while (puntero_clock < list_size(marcos)) { // Recorro hasta el size de la lista
    t_marco_asignado* marco_asignado_a_entrada = list_get(marcos, puntero_clock);
    if (marco_asignado_a_entrada->entrada->bit_uso != 0) {
      marco_asignado_a_entrada->entrada->bit_uso = 0;
    } else {
      entrada_victima = marco_asignado_a_entrada->entrada;
      puntero_clock++;
      break;
    }

    if (puntero_clock + 1 == list_size(marcos)) {
      puntero_clock = 0;
      continue;
    }
    puntero_clock++;
  }

  return entrada_victima;
}
