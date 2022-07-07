#include "libstatic.h"
#include "memoria.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/collections/dictionary.h>
#include <commons/string.h>

int CLOCK_M_NUMERO_BUSQUEDA = 1;

int obtener_y_asignar_marco_segun_algoritmo_de_reemplazo(
  int pid,
  t_entrada_tabla_segundo_nivel* entrada_segundo_nivel_solicitada_para_acceder) {
  t_list* marcos_asignados_al_proceso = obtener_marcos_asignados_a_este_proceso(pid);
  t_entrada_tabla_segundo_nivel* entrada_segundo_nivel_victima = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  int numero_marco_elegido = 0;

  xlog(COLOR_INFO, "Configurando Algoritmo de Reemplazo...");
  algoritmo_reemplazo_imprimir_marcos_asignados(pid);

  if (algoritmo_reemplazo_cargado_es("CLOCK")) {
    entrada_segundo_nivel_victima = entrada_victima_elegida_por_algoritmo_clock(
      marcos_asignados_al_proceso, entrada_segundo_nivel_solicitada_para_acceder);
    // entrada_segundo_nivel_victima = ejecutar_clock(marcos_asignados_al_proceso,
    // entrada_segundo_nivel_solicitada_para_acceder);
  } else if (algoritmo_reemplazo_cargado_es("CLOCK-M")) {
    // entrada_segundo_nivel_victima = ejecutar_clock_modificado(marcos_asignados_al_proceso, entrada);
    entrada_segundo_nivel_victima = entrada_victima_elegida_por_algoritmo_clock_modificado(
      marcos_asignados_al_proceso, entrada_segundo_nivel_solicitada_para_acceder);
  }

  numero_marco_elegido = entrada_segundo_nivel_victima->num_marco;

  // le asignamos el marco de la página víctima y la cargamos a memoria principal
  entrada_segundo_nivel_solicitada_para_acceder->num_marco = numero_marco_elegido;
  entrada_segundo_nivel_solicitada_para_acceder->bit_presencia = 1;

  // TODO: falta pasar la entrada a swap
  entrada_segundo_nivel_victima->bit_presencia = 0;

  return numero_marco_elegido;
}

// útil para diferenciar el criterio que elige este algoritmo de reemplazo como víctima
bool es_victima_segun_algoritmo_clock(t_entrada_tabla_segundo_nivel* entrada_elegida) {
  return entrada_elegida->bit_uso == 0;
}

// TODO: evaluar si remover, en la nueva implementación no lo estoy usando, fué una idea inicial
bool es_victima_segun_algoritmo_clock_modificado(t_entrada_tabla_segundo_nivel* entrada_elegida) {
  return obtener_prioridad_victima_segun_algoritmo_clock_modificado(entrada_elegida) != CLOCK_MODIFICADO_NO_ES_VICTIMA;
}

// TODO: evaluar si remover, en la nueva implementación no lo estoy usando, fué una idea inicial
CLOCK_MODIFICADO_VICTIMA_NIVEL_PRIORIDAD obtener_prioridad_victima_segun_algoritmo_clock_modificado(
  t_entrada_tabla_segundo_nivel* entrada_elegida) {
  CLOCK_MODIFICADO_VICTIMA_NIVEL_PRIORIDAD prioridad_victima = CLOCK_MODIFICADO_NO_ES_VICTIMA;

  if (entrada_elegida->bit_uso == 0 && entrada_elegida->bit_modif == 0) {
    prioridad_victima = CLOCK_MODIFICADO_VICTIMA_PRIORIDAD_ALTA;
  } else if (entrada_elegida->bit_uso == 0 && entrada_elegida->bit_modif == 1) {
    prioridad_victima = CLOCK_MODIFICADO_VICTIMA_PRIORIDAD_MEDIA;
  } else if (entrada_elegida->bit_uso == 1 && entrada_elegida->bit_modif == 1) {
    prioridad_victima = CLOCK_MODIFICADO_VICITMA_PRIORIDAD_BAJA;
  }

  return prioridad_victima;
}

void algoritmo_clock_actualizar_puntero(t_marco* marco_seleccionado, t_marco* proximo_marco_seleccionado) {
  marco_seleccionado->apuntado_por_puntero_de_clock = false;
  proximo_marco_seleccionado->apuntado_por_puntero_de_clock = true;

  xlog(COLOR_TAREA,
       "[Algoritmo Clock, NºBusqueda %d] avanzó el puntero (numero_marco_anterior=%d, numero_marco_proximo=%d)",
       CLOCK_M_NUMERO_BUSQUEDA,
       marco_seleccionado->num_marco,
       proximo_marco_seleccionado->num_marco);
}

t_marco* algoritmo_clock_puntero_obtener_marco(t_list* lista_de_marcos) {
  bool esta_apuntado_por_puntero_de_clock(t_marco * marco) {
    return marco->apuntado_por_puntero_de_clock == true;
  }

  t_marco* marco = list_find(lista_de_marcos, (void*)esta_apuntado_por_puntero_de_clock);

  return marco;
}

// TODO: mucha lógica repetida, evaluar como reducir la lógica repetida con el algoritmo clock común
t_entrada_tabla_segundo_nivel* entrada_victima_elegida_por_algoritmo_clock_modificado(
  t_list* marcos_asignados,
  t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder) {
  t_entrada_tabla_segundo_nivel* entrada_victima_elegida = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  bool victima_encontrada = false;

  t_marco* marco_apuntado_por_algoritmo_clock = algoritmo_clock_puntero_obtener_marco(marcos_asignados);

  int posicion_marco_leido = 0;
  int posicion_primer_marco_leido = 0;

  // TODO: evaluar otros escenarios, los marcos asignados por proceso siempre se devuelven ordenados
  // por numero de menor a mayor y uso su posición en la lista
  if (marco_apuntado_por_algoritmo_clock != NULL) {
    // éste lo usamos para la 1º y 2º búsqueda, se modificará sólo cuando termine alguna de las dos búsquedas (es un
    // ciclo)
    posicion_primer_marco_leido =
      obtener_posicion_de_marco_del_listado(marco_apuntado_por_algoritmo_clock, marcos_asignados);

    // si el algoritmo fué ejecutado, entonces iteramos a partir del marco al que apunte el puntero de clock
    posicion_marco_leido = posicion_primer_marco_leido;
  }

  // Criterios del clock modificado:
  // 1º busqueda: (0,0) sin modificar bit U
  // 2º busqueda: (0,1) modificando bit U
  // 3º repetir 1º búsqueda, ...

  // iterar sobre la cola circular hasta que encuentre una pagina víctima
  for (CLOCK_M_NUMERO_BUSQUEDA = 1; !victima_encontrada;) {
    for (; !victima_encontrada; posicion_marco_leido++) {
      int posicion_proximo_marco = posicion_marco_leido + 1;
      // si el marco leido es el último de la lista => el próximo marco será el primero (por ser una cola circular)
      /*if (posicion_marco_leido == list_size(marcos_asignados)) {
        posicion_proximo_marco = 0;
        posicion_marco_leido = 0;
        continue;
      }*/

      t_marco* marco_seleccionado = list_get(marcos_asignados, posicion_marco_leido);

      t_marco* proximo_marco_seleccionado = (posicion_marco_leido + 1) == list_size(marcos_asignados) ?
                                              list_get(marcos_asignados, 0) :
                                              list_get(marcos_asignados, posicion_marco_leido + 1);

      t_entrada_tabla_segundo_nivel* entrada_asignada_al_marco = marco_seleccionado->entrada_segundo_nivel;
      algoritmo_clock_entrada_imprimir_bits(entrada_asignada_al_marco);

      if (CLOCK_M_NUMERO_BUSQUEDA == 1) {
        // busqueda nº1: buscar el par (U=0, M=0)
        if (entrada_asignada_al_marco->bit_uso == 0 && entrada_asignada_al_marco->bit_modif == 0) {
          entrada_victima_elegida = entrada_asignada_al_marco;

          victima_encontrada = true;
        }
      }

      else if (CLOCK_M_NUMERO_BUSQUEDA == 2) {
        // iteramos sobre todos los marcos, evaluamos si alguno cumple (0,1) + modificamos bit U=1 (si U=1)

        if (entrada_asignada_al_marco->bit_uso == 1) {
          // podriamos siempre setear U=0, pero para respetar la teoría lo agregamos
          entrada_asignada_al_marco->bit_uso = 0;
        }
        if (entrada_asignada_al_marco->bit_uso == 0 && entrada_asignada_al_marco->bit_modif == 1) {
          entrada_victima_elegida = entrada_asignada_al_marco;

          victima_encontrada = true;
        }
      }

      // movemos el puntero del clock en la cola circular
      algoritmo_clock_actualizar_puntero(marco_seleccionado, proximo_marco_seleccionado);

      // esto facilita el uso del algoritmo de reemplazo clock, que cada marco guarde la entrada a la que apunta
      marco_seleccionado->entrada_segundo_nivel = entrada_solicitada_para_acceder;

      // volvemos al principio de la cola circular, la siguiente iteración del 2º for
      // utilizo el índice en vez de número de marco, ya que estos podrían tener cualquier valor 2,4,9,15,...
      // y ya vienen ordenados por defecto de menor a mayor (implementado en el obtener marcos asignados de un proceso)
      if (posicion_proximo_marco >= list_size(marcos_asignados)) {
        posicion_marco_leido = 0;
        posicion_proximo_marco = posicion_primer_marco_leido;
      }

      // si el próximo marco es el mismo del que partimos, entonces llegamos al final
      // (no usamos list_size porque el último de la lista no siempre es el de la última posición, podría ser el segundo
      // de la lista)
      if (posicion_proximo_marco == posicion_primer_marco_leido) {
        // alternativa usar el operador módulo para tener resultados entre (1,2)

        // si la 1º búsqueda no encontró ningún par (U=0, M=0) ==> avanzamos a la 2º búsqueda (U=0, M=1) + modificando
        // U=0 (si y sólo si U==1)
        if (CLOCK_M_NUMERO_BUSQUEDA == 1)
          CLOCK_M_NUMERO_BUSQUEDA = 2;

        // si la 2º búsqueda no encontró ningún par (U=0, M=1) ==> volvemos a la 1º búsqueda (0,0) + sin modificar U
        else if (CLOCK_M_NUMERO_BUSQUEDA == 2)
          CLOCK_M_NUMERO_BUSQUEDA = 1;
      }
    }
  }

  return entrada_victima_elegida;
}
t_entrada_tabla_segundo_nivel* entrada_victima_elegida_por_algoritmo_clock(
  t_list* marcos_asignados,
  t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder) {
  xlog(COLOR_INFO, "Ejecutando Algoritmo de Reemplazo Clock...");

  t_entrada_tabla_segundo_nivel* entrada_victima_elegida = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  bool victima_encontrada = false;

  t_marco* marco_apuntado_por_algoritmo_clock = algoritmo_clock_puntero_obtener_marco(marcos_asignados);

  int posicion_marco_leido = 0;

  // TODO: evaluar otros escenarios, los marcos asignados por proceso siempre se devuelven ordenados
  // por numero de menor a mayor y uso su posición en la lista
  if (marco_apuntado_por_algoritmo_clock != NULL) {
    // si el algoritmo fué ejecutado, entonces iteramos a partir del marco al que apunte el puntero de clock
    posicion_marco_leido = obtener_posicion_de_marco_del_listado(marco_apuntado_por_algoritmo_clock, marcos_asignados);

    xlog(COLOR_INFO,
         "[Algoritmo Clock] ya había sido ejecutado y reanuda su ejecución (posicion_marco=%d)",
         posicion_marco_leido);
  }

  // iterar sobre la cola circular hasta que encuentre una pagina víctima
  for (; !victima_encontrada; posicion_marco_leido++) {
    t_marco* marco_seleccionado = list_get(marcos_asignados, posicion_marco_leido);
    int posicion_proximo_marco = posicion_marco_leido + 1;
    int cantidad_marcos_asignados = list_size(marcos_asignados) - 1;

    xlog(COLOR_INFO,
         "[Algoritmo Clock] analiza posicion_marco_leido (%d) >= cantidad_marcos_asignados (%d)",
         posicion_marco_leido,
         cantidad_marcos_asignados);

    // si el marco leido es el último de la lista => el próximo marco será el primero (por ser una cola circular)
    if (posicion_marco_leido >= cantidad_marcos_asignados) {
      posicion_proximo_marco = 0;

      xlog(COLOR_TAREA,
           "[Algoritmo Clock] el próximo marco será el primero de la lista (posicion_marco=%d)",
           posicion_marco_leido);
    }

    t_marco* proximo_marco_seleccionado = list_get(marcos_asignados, posicion_proximo_marco);
    t_entrada_tabla_segundo_nivel* entrada_asignada_al_marco = marco_seleccionado->entrada_segundo_nivel;

    algoritmo_reemplazo_imprimir_marco(marco_seleccionado);
    algoritmo_reemplazo_imprimir_entrada_segundo_nivel(entrada_asignada_al_marco);

    if (!es_victima_segun_algoritmo_clock(entrada_asignada_al_marco)) {
      // le damos otra oportunidad
      entrada_asignada_al_marco->bit_uso = 0;

      xlog(COLOR_TAREA,
           "[Algoritmo Clock] detectó un marco con bit_uso=1, le dió otra oportunidad (posicion_marco=%d)",
           posicion_marco_leido);
    } else {
      entrada_victima_elegida = entrada_asignada_al_marco;

      // corta el flujo de iteración del for, no usamos un break/return para poder actualizar el puntero del clock
      victima_encontrada = true;

      xlog(COLOR_TAREA,
           "[Algoritmo Clock] detectó una entrada víctima (posicion_marco=%d, entrada_numero=%d)",
           posicion_marco_leido,
           entrada_victima_elegida->entrada_segundo_nivel);
    }

    algoritmo_clock_actualizar_puntero(marco_seleccionado, proximo_marco_seleccionado);

    // esto facilita el uso del algoritmo de reemplazo clock, que cada marco guarde la entrada a la que apunta
    marco_seleccionado->entrada_segundo_nivel = entrada_solicitada_para_acceder;

    xlog(COLOR_INFO,
         "[Algoritmo Clock] analiza posicion_proximo_marco (%d) >= cantidad_marcos_asignados (%d)",
         posicion_marco_leido,
         cantidad_marcos_asignados);

    // volvemos al principio de la cola circular, el for seguirá iterando
    if (posicion_proximo_marco >= cantidad_marcos_asignados) {
      posicion_marco_leido = 0;

      xlog(COLOR_TAREA,
           "[Algoritmo Clock] volver al principio de la cola circular (posicion_marco=%d)",
           posicion_proximo_marco);
    }

    printf("\n");
  }

  return entrada_victima_elegida;
}

// TODO: validar si contempla todos los escenarios, por el momento estoy usando
// entrada_victima_elegida_por_algoritmo_clock
t_entrada_tabla_segundo_nivel* ejecutar_clock(t_list* marcos_asignados,
                                              t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder) {
  t_entrada_tabla_segundo_nivel* entrada_victima = malloc(sizeof(t_entrada_tabla_segundo_nivel));

  while (puntero_clock < list_size(marcos_asignados)) {
    t_marco_asignado* marco_asignado_a_entrada = list_get(marcos_asignados, puntero_clock);

    if (marco_asignado_a_entrada->entrada->bit_uso != 0) {
      marco_asignado_a_entrada->entrada->bit_uso = 0;
    } else {
      entrada_victima = marco_asignado_a_entrada->entrada;
      puntero_clock++;
      break;
    }

    if (puntero_clock + 1 == list_size(marcos_asignados)) {
      puntero_clock = 0;
      continue;
    }
    puntero_clock++;
  }

  return entrada_victima;
}
