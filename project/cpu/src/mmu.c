#include "mmu.h"

// int main() {
//   prueba_datos_tlb();
// }

void iniciar_tlb() {
  tlb = list_create();
  cantidad_entradas_tlb = config_get_int_value(config, "ENTRADAS_TLB");
  reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
}

void limpiar_tlb() {
  list_clean(tlb);
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

t_entrada_tlb* obtener_entrada_tlb(uint32_t pagina, uint32_t marco) {
  t_entrada_tlb* retorno = malloc(sizeof(t_entrada_tlb));

  retorno->pagina = pagina;
  retorno->marco = marco;
  retorno->time = time(NULL);

  return retorno;
}

void agregar_entrada_tlb(t_entrada_tlb* entrada_tlb) {
  if (list_size(tlb) == cantidad_entradas_tlb) {
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

  while (index < (list_size(tlb) - 1)) {
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
  uint64_t aux_time = UINT64_MAX;

  while (index < (list_size(tlb) - 1)) {
    t_entrada_tlb* entrada_tlb = list_get(tlb, index);
    if (entrada_tlb->time < aux_time) {
      index_buscado = index;
      aux_time = entrada_tlb->time;
    }
    index++;
  }

  return index_buscado;
}

// void reemplazo_fifo(t_entrada_tlb* entrada_reemplazo) {
//   while (puntero_reemplazo < list_size(tlb) - 1) {
//     t_entrada_tlb* entrada_actual = list_get(tlb, puntero_reemplazo);
//     list_replace(tlb, puntero_reemplazo, entrada_actual);
//     puntero_reemplazo++;
//   }

//   if (puntero_reemplazo == list_size(tlb) - 1) {
//     puntero_reemplazo = 0;
//   }
// }

// void reemplazo_lru() {
//   t_entrada_tlb* entrada_buscada = malloc(sizeof(t_entrada_tlb));
//   uint64_t aux_timestamp = UINT64_MAX;

//   void search_oldest(void* elemento) {
//     t_entrada_tlb* entrada = (t_entrada_tlb*)elemento;

//     if (entrada->timestamp < aux_timestamp) {
//       aux_timestamp = entrada->timestamp;
//       entrada_buscada = entrada;
//     }
//   }

//   list_iterate(tlb, search_oldest);
// }

void prueba_datos_tlb() {
  t_entrada_tlb* entrada1 = malloc(sizeof(t_entrada_tlb));
  t_entrada_tlb* entrada2 = malloc(sizeof(t_entrada_tlb));
  t_entrada_tlb* entrada3 = malloc(sizeof(t_entrada_tlb));
  t_entrada_tlb* entrada4 = malloc(sizeof(t_entrada_tlb));

  entrada1->pagina = 1;
  entrada1->marco = 1;
  entrada1->time = time(NULL);

  entrada2->pagina = 2;
  entrada2->marco = 2;
  entrada2->time = time(NULL);

  entrada3->pagina = 3;
  entrada3->marco = 3;
  entrada3->time = time(NULL);

  entrada4->pagina = 4;
  entrada4->marco = 4;
  entrada4->time = time(NULL);

  list_add(tlb, entrada1);
  list_add(tlb, entrada2);
  list_add(tlb, entrada3);
  list_add(tlb, entrada4);
}