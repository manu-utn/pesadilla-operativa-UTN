#include "mmu.h"

// int main() {
//   t_list* lista = prueba_crear_datos_tlb();
//   reemplazo_tlb = "LRU";
//   realizar_pruebas_tlb(lista);
// }

void iniciar_tlb() {
  tlb = list_create();
  cantidad_entradas_tlb = config_get_int_value(config, "ENTRADAS_TLB");
  reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
}

void limpiar_tlb(u_int32_t pid) {
  if (list_size(tlb) == 0)
    return;

  t_entrada_tlb* entrada_tlb = list_get(tlb, 0);
  if (pid != entrada_tlb->pid) {
    list_clean(tlb);
  }
}

uint32_t obtener_numero_pagina(uint32_t direccion_logica) {
  return (float)(direccion_logica / tamanio_pagina);
}

uint32_t obtener_entrada_1er_nivel(uint32_t numero_pagina, uint32_t entradas_por_tabla) {
  return (float)(numero_pagina / entradas_por_tabla);
}

uint32_t obtener_entrada_2do_nivel(uint32_t numero_pagina, uint32_t entradas_por_tabla) {
  return (numero_pagina % entradas_por_tabla);
}

uint32_t obtener_desplazamiento(uint32_t direccion_logica, uint32_t numero_pagina) {
  return (direccion_logica - (numero_pagina * tamanio_pagina));
}

uint32_t obtener_direccion_fisica(uint32_t desplazamiento, uint32_t marco) {
  return (marco * tamanio_pagina + desplazamiento);
}

uint32_t obtener_marco_tlb(int indice) {
  t_entrada_tlb* entrada_tlb = list_get(tlb, indice);
  uint32_t marco = entrada_tlb->marco;
  free(entrada_tlb);
  return marco;
}

void agregar_pagina_marco_tlb(uint32_t pagina, uint32_t marco, uint32_t pid) {
  t_entrada_tlb* entrada_tlb = obtener_entrada_tlb(pagina, marco, pid);
  agregar_entrada_tlb(entrada_tlb);
}

t_entrada_tlb* obtener_entrada_tlb(uint32_t pagina, uint32_t marco, uint32_t pid) {
  t_entrada_tlb* retorno = malloc(sizeof(t_entrada_tlb));

  retorno->pid = pid;
  retorno->pagina = pagina;
  retorno->marco = marco;

  struct timespec timestamp;
  clock_gettime(CLOCK_REALTIME, &timestamp);
  retorno->time_sec = timestamp.tv_sec;
  retorno->time_nanosec = timestamp.tv_nsec;

  return retorno;
}

void agregar_entrada_tlb(t_entrada_tlb* entrada_tlb) {
  if (list_size(tlb) == cantidad_entradas_tlb || existe_pagina_en_tlb(entrada_tlb->pagina) != -1) {
    if (strcmp(reemplazo_tlb, "LRU") == 0) {
      realizar_reemplazo_lru(entrada_tlb);
    } else {
      realizar_reemplazo_fifo(entrada_tlb);
    }
  } else {
    list_add(tlb, entrada_tlb);
  }
}

void realizar_reemplazo_fifo(t_entrada_tlb* entrada_tlb) {
  int existe_pagina = existe_pagina_en_tlb(entrada_tlb->pagina);

  if (existe_pagina != -1) {
    return;
  } else {
    int index_buscado = busco_index_oldest();
    void* entrada_tlb_reemplazada = list_replace(tlb, index_buscado, entrada_tlb);
    free(entrada_tlb_reemplazada);
  }
}

void realizar_reemplazo_lru(t_entrada_tlb* entrada_tlb) {
  void* entrada_tlb_reemplazada;

  int existe_pagina = existe_pagina_en_tlb(entrada_tlb->pagina);

  if (existe_pagina != -1) {
    entrada_tlb_reemplazada = list_replace(tlb, existe_pagina, entrada_tlb);
  } else {
    int index_buscado = busco_index_oldest();
    entrada_tlb_reemplazada = list_replace(tlb, index_buscado, entrada_tlb);
  }
  free(entrada_tlb_reemplazada);
}

int existe_pagina_en_tlb(uint32_t pagina) {
  int index = 0;

  while (index < list_size(tlb)) {
    t_entrada_tlb* entrada_tlb = list_get(tlb, index);
    if (entrada_tlb->pagina == pagina) {
      return index;
    }
    index++;
  }

  return -1;
}

int busco_index_oldest() {
  int index = 0;
  int index_buscado = -1;

  // cambiamos el tipo y el valor de los uint de aux_time y aux_time_micro,
  // estos provocaban error en momento de enlazado (gnu link)
  uint32_t aux_time = 4294967295;
  uint32_t aux_time_micro = 4294967295;

  while (index < list_size(tlb)) {
    t_entrada_tlb* entrada_tlb = list_get(tlb, index);
    if (entrada_tlb->time_sec < aux_time) {
      index_buscado = index;
      aux_time = entrada_tlb->time_sec;
      aux_time_micro = entrada_tlb->time_nanosec;
    } else if (entrada_tlb->time_sec == aux_time) {
      if (entrada_tlb->time_nanosec < aux_time_micro) {
        index_buscado = index;
        aux_time = entrada_tlb->time_sec;
        aux_time_micro = entrada_tlb->time_nanosec;
      }
    }
    index++;
  }

  return index_buscado;
}

t_list* prueba_crear_datos_tlb() {
  tlb = list_create();
  cantidad_entradas_tlb = 3;
  t_entrada_tlb* entrada1 = obtener_entrada_tlb(2, 2, 1);
  t_entrada_tlb* entrada2 = obtener_entrada_tlb(3, 3, 1);
  t_entrada_tlb* entrada3 = obtener_entrada_tlb(2, 2, 1);
  t_entrada_tlb* entrada4 = obtener_entrada_tlb(1, 1, 1);
  t_entrada_tlb* entrada5 = obtener_entrada_tlb(5, 5, 1);
  t_entrada_tlb* entrada6 = obtener_entrada_tlb(2, 2, 1);
  t_entrada_tlb* entrada7 = obtener_entrada_tlb(4, 4, 1);
  t_entrada_tlb* entrada8 = obtener_entrada_tlb(5, 5, 1);
  t_entrada_tlb* entrada9 = obtener_entrada_tlb(3, 3, 1);
  t_entrada_tlb* entrada10 = obtener_entrada_tlb(2, 2, 1);
  t_entrada_tlb* entrada11 = obtener_entrada_tlb(5, 5, 1);
  t_entrada_tlb* entrada12 = obtener_entrada_tlb(2, 2, 1);

  t_list* tlb_prueba = list_create();

  list_add(tlb_prueba, entrada1);
  list_add(tlb_prueba, entrada2);
  list_add(tlb_prueba, entrada3);
  list_add(tlb_prueba, entrada4);
  list_add(tlb_prueba, entrada5);
  list_add(tlb_prueba, entrada6);
  list_add(tlb_prueba, entrada7);
  list_add(tlb_prueba, entrada8);
  list_add(tlb_prueba, entrada9);
  list_add(tlb_prueba, entrada10);
  list_add(tlb_prueba, entrada11);
  list_add(tlb_prueba, entrada12);
  return tlb_prueba;
}

void realizar_pruebas_tlb(t_list* tlb_prueba) {
  for (int i = 0; i < list_size(tlb_prueba); i++) {
    t_entrada_tlb* entrada = list_get(tlb_prueba, i);
    agregar_entrada_tlb(entrada);
  }
}
