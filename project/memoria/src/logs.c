#include "memoria.h"

void imprimir_marco(t_marco* marco) {
  xlog(COLOR_INFO,
       "[MARCO] numero=%d, pid=%d, ocupado=%s, numero_entrada_segundo_nivel=%d, apuntado_por_algoritmo_clock=%s",
       marco->num_marco,
       marco->pid,
       (marco->ocupado) ? "SI" : "NO",
       (marco->entrada_segundo_nivel) ? marco->entrada_segundo_nivel->entrada_segundo_nivel : -1,
       (marco->apuntado_por_puntero_de_clock) ? "SI" : "NO");

  if (marco->entrada_segundo_nivel) {
    t_entrada_tabla_segundo_nivel* entrada_asignada_al_marco = marco->entrada_segundo_nivel;
    entrada_asignada_a_marco_imprimir_bits(entrada_asignada_al_marco);
  }
}

void algoritmo_reemplazo_imprimir_marco(t_marco* marco) {
  xlog(COLOR_INFO,
       "[Algoritmo Reemplazo] [MARCO] numero_marco=%d, pid=%d, ocupado=%s, numero_entrada_segundo_nivel=%d",
       marco->num_marco,
       marco->pid,
       (marco->ocupado) ? "SI" : "NO",
       (marco->entrada_segundo_nivel) ? marco->entrada_segundo_nivel->entrada_segundo_nivel : -1);
}

void mostrar_tabla_marcos() {
  xlog(COLOR_INFO, "Imprimiendo datos de los marcos en memoria...");

  list_iterate(tabla_marcos, (void*)imprimir_marco);
}

void algoritmo_reemplazo_imprimir_marcos_asignados(int pid) {
  xlog(COLOR_INFO, "[Algoritmo Reemplazo] Imprimiendo datos de los marcos asignados a un proceso... (pid=%d)", pid);

  t_list* marcos_asignados_al_proceso = obtener_marcos_asignados_a_este_proceso(pid);
  list_iterate(marcos_asignados_al_proceso, (void*)imprimir_marco);
}

void algoritmo_reemplazo_imprimir_entrada_segundo_nivel(t_entrada_tabla_segundo_nivel* entrada) {
  if (algoritmo_reemplazo_cargado_es("CLOCK")) {
    xlog(COLOR_INFO,
         "[Algoritmo Reemplazo] [ENTRADA] numero=%d, marco=%d, bit_de_uso=%d, bit_de_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_presencia);
  } else if (algoritmo_reemplazo_cargado_es("CLOCK-M")) {
    xlog(COLOR_INFO,
         "[Algoritmo Reemplazo] [ENTRADA] numero=%d, marco=%d, bit_de_uso=%d, bit_de_modificado=%d, "
         "bit_de_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_modif,
         entrada->bit_presencia);
  }
}

void imprimir_entrada_segundo_nivel(char* __, t_entrada_tabla_segundo_nivel* entrada) {
  if (algoritmo_reemplazo_cargado_es("CLOCK")) {
    xlog(COLOR_INFO,
         "....[TP_SEGUNDO_NIVEL] entrada_numero=%d, numero_marco=%d, bit_de_uso=%d, "
         "bit_de_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_presencia);
  } else if (algoritmo_reemplazo_cargado_es("CLOCK-M")) {
    xlog(COLOR_INFO,
         "....[TP_SEGUNDO_NIVEL] entrada_numero=%d, numero_marco=%d, bit_de_uso=%d, bit_de_modificado=%d, "
         "bit_de_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_modif,
         entrada->bit_presencia);
  }
}

void imprimir_entradas_tabla_paginas_segundo_nivel(t_tabla_segundo_nivel* tabla_segundo_nivel) {
  xlog(COLOR_INFO, "[TP_SEGUNDO_NIVEL] tp_numero=%d, cantidad_entradas=%d", tabla_segundo_nivel->num_tabla, dictionary_size(tabla_segundo_nivel->entradas_segundo_nivel));

  void imprimir_entrada_segundo_nivel(char* ___, t_entrada_tabla_segundo_nivel* entrada) {
    xlog(COLOR_INFO,
         "..[ENTRADA_SEGUNDO_NIVEL] numero=%d, marco=%d, bit_uso=%d, bit_modificado=%d, bit_presencia=%d",
         entrada->entrada_segundo_nivel,
         entrada->num_marco,
         entrada->bit_uso,
         entrada->bit_modif,
         entrada->bit_presencia);
  }

  dictionary_iterator(tabla_segundo_nivel->entradas_segundo_nivel, (void*)imprimir_entrada_segundo_nivel);
}

void imprimir_tabla_paginas_primer_nivel(char* __, t_tabla_primer_nivel* tabla_primer_nivel) {
  xlog(COLOR_INFO,
       "[TP_PRIMER_NIVEL] tp_numero=%d, pid=%d, cantidad_entradas=%d",
       tabla_primer_nivel->num_tabla,
       tabla_primer_nivel->pid,
       dictionary_size(tabla_primer_nivel->entradas_primer_nivel));

  void imprimir_entrada_primer_segundo_nivel(char* ___, t_entrada_tabla_primer_nivel* entrada_primer_nivel) {
    t_tabla_segundo_nivel* tabla_segundo_nivel = obtener_TP_segundo_nivel(tabla_primer_nivel->num_tabla, entrada_primer_nivel->entrada_primer_nivel);

    xlog(COLOR_INFO,
         "..[ENTRADA_PRIMER_NIVEL] entrada_numero=%d, tp_segundo_nivel_numero=%d, pid=%d, cantidad_entradas=%d",
         entrada_primer_nivel->entrada_primer_nivel,
         tabla_segundo_nivel->num_tabla,
         tabla_segundo_nivel->pid,
         dictionary_size(tabla_segundo_nivel->entradas_segundo_nivel));

    dictionary_iterator(tabla_segundo_nivel->entradas_segundo_nivel, (void*)imprimir_entrada_segundo_nivel);
  }

  dictionary_iterator(tabla_primer_nivel->entradas_primer_nivel, (void*)imprimir_entrada_primer_segundo_nivel);
}

void imprimir_tablas_de_paginas() {
  xlog(COLOR_INFO, "Imprimiendo datos de las tablas de paginas...");

  dictionary_iterator(tablas_de_paginas_primer_nivel, (void*)imprimir_tabla_paginas_primer_nivel);
}

void algoritmo_clock_entrada_imprimir_bits(t_entrada_tabla_segundo_nivel* entrada) {
  xlog(COLOR_INFO,
       "[Algoritmo Reemplazo] [ENTRADA] numero=%d, marco=%d, bit_uso=%d, bit_modificado=%d, bit_presencia=%d",
       entrada->entrada_segundo_nivel,
       entrada->num_marco,
       entrada->bit_uso,
       entrada->bit_modif,
       entrada->bit_presencia);
}

void entrada_asignada_a_marco_imprimir_bits(t_entrada_tabla_segundo_nivel* entrada) {
  xlog(COLOR_INFO,
       "[MARCO] [ENTRADA] tp_segundo_nivel_numero=%d, entrada_numero=%d, marco=%d, bit_uso=%d, bit_modificado=%d, bit_presencia=%d",
       entrada->numero_tabla_segundo_nivel,
       entrada->entrada_segundo_nivel,
       entrada->num_marco,
       entrada->bit_uso,
       entrada->bit_modif,
       entrada->bit_presencia);
}
