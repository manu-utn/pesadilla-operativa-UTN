#include "memoria.h"

void simular_solicitud_marco_por_mmu() {
  int marco = 0;

  // simulación para OPERACION_OBTENER_MARCO en memoria.c
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

  numero_tp_segundo_nivel = 2;
  printf("\n");
  marco = obtener_marco(numero_tp_segundo_nivel, 2);
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
  entrada->bit_uso = 1, entrada->bit_modif = 0; // u=1, m=0
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
