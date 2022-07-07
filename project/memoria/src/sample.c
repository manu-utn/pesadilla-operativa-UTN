#include "sample.h"
#include "memoria.h"
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>
#include <utils.h>

int main() {
  // función de la biblioteca static
  inicializar_estructuras();
  char* path_punto_monateje = config_get_string_value(config, "PATH_SWAP");
  crear_punto_de_montaje(path_punto_monateje);
  // pthread_t th;
  // pthread_create(&th, NULL, escuchar_conexiones, NULL), pthread_detach(th);

  // PARA TESTEAR ALGORITMOS DE REEMPLAZO
  // reservar_marcos_mock();

  inicializar_estructuras_de_este_proceso(0, 500);

  mostrar_tabla_marcos();
  imprimir_tablas_de_paginas();

  // reservar_marcos_2();
  reservar_marcos_3();

  simular_solicitud_marco_por_mmu();

  // comentamos para probar la otra versión

  /*
    t_entrada_tabla_segundo_nivel* entrada = malloc(sizeof(t_entrada_tabla_segundo_nivel));

    entrada->entrada_segundo_nivel = 5;
    entrada->num_marco = -1;
    entrada->bit_uso = 0;
    entrada->bit_modif = 0;
    entrada->bit_presencia = 0;

    // int marco_victima = ejecutar_clock(marcos_prueba_clock, entrada);
    t_entrada_tabla_segundo_nivel* entradaVictima =
      entrada_victima_elegida_por_algoritmo_clock_modificado(marcos_prueba_clock, entrada);
  */

  return 0;
}


void simular_solicitud_marco_por_mmu() {
  int marco = 0;

  // configuración de la simulación para memoria con 4 frames

  int numero_tp_segundo_nivel;
  // estas peticiones encontrará el frame asignado (pid=1)
  numero_tp_segundo_nivel = 1;
  marco = obtener_marco(numero_tp_segundo_nivel, 0);
  marco = obtener_marco(numero_tp_segundo_nivel, 1);
  marco = obtener_marco(numero_tp_segundo_nivel, 2);
  marco = obtener_marco(numero_tp_segundo_nivel, 3);
  printf("\n");

  numero_tp_segundo_nivel = 2;
  marco = obtener_marco(numero_tp_segundo_nivel, 0);

  /*
  // en estas cuatro peticiones buscará el primer marco libre asignado al proceso
  nuevo_marco = obtener_marco(1, 1);
  nuevo_marco = obtener_marco(1, 2);
  nuevo_marco = obtener_marco(1, 3);
  nuevo_marco = obtener_marco(1, 4);
  printf("\n");

  // en estas dos peticiones buscará el marco ya asignado a esas entradas
  nuevo_marco = obtener_marco(1, 1);
  nuevo_marco = obtener_marco(1, 2);
  printf("\n");

  // en estas dos peticiones ejecutará el algoritmo de reemplazo,
  // con la configuración actual el proceso tiene asignado 4 marcos, y ya usó 4
  nuevo_marco = obtener_marco(2, 1);
  nuevo_marco = obtener_marco(2, 2);
   */
}
