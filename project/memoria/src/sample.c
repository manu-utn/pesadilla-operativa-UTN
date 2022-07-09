#include "sample.h"
#include "libstatic.h" // <-- STATIC LIB
#include "memoria.h"
#include <stdio.h>
#include <utils.h>

int main() {
  inicializar_estructuras();

  // TODO: comentado temporalmente, pido que se utilicen las macros para las rutas
  /*
  char* path_punto_monataje = config_get_string_value(config, "PATH_SWAP");
  crear_punto_de_montaje(path_punto_monataje);
  inicializar_archivo_swap(0, 160, path_punto_monataje);
  inicializar_archivo_swap(1, 160, path_punto_monataje);

  void* textoPrueba = "123Probaasasa";
  void* textoPrueba2 = "caca";

  escribir_archivo_swap("/home/utnso/swap/0.swap", textoPrueba, 2, 32);
  escribir_archivo_swap("/home/utnso/swap/0.swap", textoPrueba2, 1, 32);
  eliminar_archivo_swap(1, path_punto_monataje);
  leer_archivo_swap("/home/utnso/swap/0.swap", 2, 32);
   */

  inicializar_estructuras_de_este_proceso(0, 500);

  mostrar_tabla_marcos();
  imprimir_tablas_de_paginas();

  // simular_asignacion_marcos_1();
  simular_asignacion_marcos_2();

  simular_solicitud_marco_por_mmu();

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
  // marco = obtener_marco(numero_tp_segundo_nivel, 1);

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

void simular_asignacion_marcos_1() {
  t_tabla_segundo_nivel* tabla = dictionary_get(tablas_de_paginas_segundo_nivel, "1");
  t_entrada_tabla_segundo_nivel* entrada;

  int pid, numero_marco, numero_entrada;
  pid = 1;

  xlog(COLOR_INFO, "[MOCK] Reservando marcos (TP_1er_nivel=%d, pid=%d)...", tabla->num_tabla, pid);

  // Mock de Marcos asignados
  //
  // frame 0: PID_1-Entrada_0 (u=1, m=0)
  // frame 1: PID_1-Entrada_1 (u=1, m=1, puntero_clock=true) <- debería de ser la víctima para clock-modificado
  // frame 2: PID_1-Entrada_2 (u=1, m=0)
  // frame 3: PID_1-Entrada_3 (u=1, m=0)

  numero_marco = 0, numero_entrada = 0;
  // t_marco* marco1 = marco_create(1, pid, MARCO_OCUPADO);
  entrada = obtener_entrada_tabla_segundo_nivel(tabla->num_tabla, numero_entrada);
  entrada->bit_uso = 1, entrada->bit_modif = 0; // u=1, m=0
  reasignar_marco(numero_marco, pid, entrada);
  xlog(COLOR_INFO, "[MOCK] Marco reasignado (num_marco=%d, pid=%d, numero_entrada=%d)", numero_marco, pid, numero_entrada);

  numero_marco = 1, numero_entrada = 1;
  entrada = obtener_entrada_tabla_segundo_nivel(tabla->num_tabla, numero_entrada);
  entrada->bit_uso = 1, entrada->bit_modif = 1; // u=1, m=1
  // TODO: evaluar que ocurriría si el puntero no apunta a ningun marco, por el momento funciona bien así
  // algoritmo_clock_puntero_apuntar_al_marco(numero_marco);
  reasignar_marco(numero_marco, pid, entrada);
  xlog(COLOR_INFO, "[MOCK] Marco reasignado (num_marco=%d, pid=%d, numero_entrada=%d)", numero_marco, pid, numero_entrada);

  numero_marco = 2, numero_entrada = 2;
  entrada = obtener_entrada_tabla_segundo_nivel(tabla->num_tabla, numero_entrada);
  entrada->bit_uso = 1, entrada->bit_modif = 0; // u=1, m=0
  reasignar_marco(numero_marco, pid, entrada);
  xlog(COLOR_INFO, "[MOCK] Marco reasignado (num_marco=%d, pid=%d, numero_entrada=%d)", numero_marco, pid, numero_entrada);

  numero_marco = 3, numero_entrada = 3;
  entrada = obtener_entrada_tabla_segundo_nivel(tabla->num_tabla, numero_entrada);
  entrada->bit_uso = 1, entrada->bit_modif = 0; // u=1, m=0
  algoritmo_clock_puntero_apuntar_al_marco(numero_marco);
  reasignar_marco(numero_marco, pid, entrada);
  xlog(COLOR_INFO, "[MOCK] Marco reasignado (num_marco=%d, pid=%d, numero_entrada=%d)", numero_marco, pid, numero_entrada);

  imprimir_entradas_tabla_paginas_segundo_nivel(tabla);
}

void simular_asignacion_marcos_2() {
  t_tabla_segundo_nivel* tabla = dictionary_get(tablas_de_paginas_segundo_nivel, "1");
  t_entrada_tabla_segundo_nivel* entrada;

  int pid, numero_marco, numero_entrada;
  pid = 1;

  xlog(COLOR_INFO, "[MOCK] Reservando marcos (TP_1er_nivel=%d, pid=%d)...", tabla->num_tabla, pid);

  // Mock de Marcos asignados
  //
  // frame 0: PID_1-Entrada_0 (u=1, m=1)
  // frame 1: PID_1-Entrada_1 (u=1, m=1, puntero_clock=true)
  // frame 2: PID_1-Entrada_2 (u=1, m=0)
  // frame 3: PID_1-Entrada_3 (u=0, m=0) <- debería de ser la víctima para clock-modificado

  numero_marco = 0, numero_entrada = 0;
  // t_marco* marco1 = marco_create(1, pid, MARCO_OCUPADO);
  entrada = obtener_entrada_tabla_segundo_nivel(tabla->num_tabla, numero_entrada);
  entrada->bit_uso = 1, entrada->bit_modif = 1; // u=1, m=1
  reasignar_marco(numero_marco, pid, entrada);
  xlog(COLOR_INFO, "[MOCK] Marco reasignado (num_marco=%d, pid=%d, numero_entrada=%d)", numero_marco, pid, numero_entrada);

  numero_marco = 1, numero_entrada = 1;
  entrada = obtener_entrada_tabla_segundo_nivel(tabla->num_tabla, numero_entrada);
  entrada->bit_uso = 1, entrada->bit_modif = 1; // u=1, m=1
  // TODO: evaluar que ocurriría si el puntero no apunta a ningun marco, por el momento funciona bien así
  algoritmo_clock_puntero_apuntar_al_marco(numero_marco);
  reasignar_marco(numero_marco, pid, entrada);
  xlog(COLOR_INFO, "[MOCK] Marco reasignado (num_marco=%d, pid=%d, numero_entrada=%d)", numero_marco, pid, numero_entrada);

  numero_marco = 2, numero_entrada = 2;
  entrada = obtener_entrada_tabla_segundo_nivel(tabla->num_tabla, numero_entrada);
  entrada->bit_uso = 1, entrada->bit_modif = 0; // u=1, m=0
  reasignar_marco(numero_marco, pid, entrada);
  xlog(COLOR_INFO, "[MOCK] Marco reasignado (num_marco=%d, pid=%d, numero_entrada=%d)", numero_marco, pid, numero_entrada);

  numero_marco = 3, numero_entrada = 3;
  entrada = obtener_entrada_tabla_segundo_nivel(tabla->num_tabla, numero_entrada);
  entrada->bit_uso = 0, entrada->bit_modif = 0; // u=0, m=0
  reasignar_marco(numero_marco, pid, entrada);
  xlog(COLOR_INFO, "[MOCK] Marco reasignado (num_marco=%d, pid=%d, numero_entrada=%d)", numero_marco, pid, numero_entrada);

  imprimir_entradas_tabla_paginas_segundo_nivel(tabla);
}
