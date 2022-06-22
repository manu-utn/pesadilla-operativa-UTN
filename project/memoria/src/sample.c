#include "sample.h"
#include "memoria.h"
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>
#include <utils.h>

int main() {
  // función de la biblioteca static
  inicializar_estructuras();
  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexiones, NULL), pthread_detach(th);

  // PARA TESTEAR ALGORITMOS DE REEMPLAZO
  // reservar_marcos_mock();

  // inicializar_estructuras_de_este_proceso(0, 500);

  // mostrar_tabla_marcos();
  // imprimir_tablas_de_paginas();

  // simular_solicitud_marco_por_mmu();

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
    entrada_victima_elegida_por_algoritmo_clock(marcos_prueba_clock, entrada);
   *
   */

  xlog(COLOR_INFO, "CPU - Servidor listo para recibir al cliente Kernel");

  pthread_exit(0);
  // return 0;
}

void simular_solicitud_marco_por_mmu() {
  int nuevo_marco = 0;
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
}
