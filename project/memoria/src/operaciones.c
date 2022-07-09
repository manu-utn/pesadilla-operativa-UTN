#include "libstatic.h"
#include "memoria.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/collections/dictionary.h>
#include <commons/string.h>


uint32_t escribir_dato(uint32_t direccion_fisica, uint32_t valor) {
  uint32_t byte_inicio = obtener_byte_inicio(direccion_fisica);
  memcpy(memoria_principal + byte_inicio, &valor, sizeof(uint32_t));

  xlog(COLOR_INFO,
       "OPERACION - Escribir dato (%d) en memoria principal realizada correctamente. Direccion fisica: %d.",
       valor,
       direccion_fisica);
  return 1;
}

uint32_t buscar_dato_en_memoria(uint32_t direccion_fisica) {
  uint32_t byte_inicio = obtener_byte_inicio(direccion_fisica);
  uint32_t dato_buscado = 0;
  memcpy(&dato_buscado, memoria_principal + byte_inicio, sizeof(uint32_t));

  xlog(COLOR_INFO,
       "OPERACION - Buscar dato en memoria principal realizada correctamente. Dato buscado: %d, Direccion fisica: %d.",
       dato_buscado,
       direccion_fisica);
  return dato_buscado;
}

uint32_t obtener_byte_inicio(uint32_t direccion_fisica) {
  uint32_t marco_buscado = obtener_marco_dato(direccion_fisica);
  uint32_t offset_buscado = obtener_offset_dato(direccion_fisica);
  return (marco_buscado * tamanio_marco) + offset_buscado;
}

uint32_t obtener_marco_dato(uint32_t direccion_fisica) {
  return direccion_fisica / tamanio_marco;
}

uint32_t obtener_offset_dato(uint32_t direccion_fisica) {
  return direccion_fisica % tamanio_marco;
}
