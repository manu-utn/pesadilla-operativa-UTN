#include "memoria.h"

uint32_t escribir_dato(uint32_t direccion_fisica, uint32_t valor) {
  uint32_t byte_inicio = obtener_byte_inicio(direccion_fisica, 1); // 1 para modificar bit_modif
  memcpy(memoria_principal + byte_inicio, &valor, sizeof(uint32_t));

  xlog(COLOR_INFO, "OPERACION - Escribir dato (%d) en memoria principal realizada correctamente. Direccion fisica: %d.", valor, direccion_fisica);
  return 1;
}

uint32_t buscar_dato_en_memoria(uint32_t direccion_fisica) {
  uint32_t byte_inicio = obtener_byte_inicio(direccion_fisica, 0); // 0 para no modificar bit_modif
  uint32_t dato_buscado = 0;
  memcpy(&dato_buscado, memoria_principal + byte_inicio, sizeof(uint32_t));

  xlog(COLOR_INFO, "OPERACION - Buscar dato en memoria principal realizada correctamente. Dato buscado: %d, Direccion fisica: %d.", dato_buscado, direccion_fisica);
  return dato_buscado;
}

uint32_t obtener_byte_inicio(uint32_t direccion_fisica, uint32_t bit_modificado) {
  uint32_t marco_buscado = obtener_marco_dato(direccion_fisica);
  uint32_t offset_buscado = obtener_offset_dato(direccion_fisica);

  actualizar_bits(marco_buscado, bit_modificado);

  return (marco_buscado * obtener_tamanio_pagina_por_config()) + offset_buscado;
}

uint32_t obtener_marco_dato(uint32_t direccion_fisica) {
  return direccion_fisica / obtener_tamanio_pagina_por_config();
}

uint32_t obtener_offset_dato(uint32_t direccion_fisica) {
  return direccion_fisica % obtener_tamanio_pagina_por_config();
}

void actualizar_bits(uint32_t numero_marco, uint32_t bit_modificado) {
  int index = 0;

  while (index < list_size(tabla_marcos)) {
    t_marco* marco = list_get(tabla_marcos, index);
    if (marco->num_marco == numero_marco) {
      marco->entrada_segundo_nivel->bit_uso = 1;
      if (bit_modificado == 1) {
        marco->entrada_segundo_nivel->bit_modif = 1;
      }
      return;
    }
    index++;
  }

  return;
}
