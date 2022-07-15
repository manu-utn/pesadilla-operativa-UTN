#include "libstatic.h"
#include "memoria.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/collections/dictionary.h>
#include <commons/string.h>

// TODO: validar
int escribir_dato(uint32_t dir_fisica, uint32_t valor) {
  // Busco a que proceso pertenece

  int resultado = 0;

  return resultado;
}


// TODO: validar
uint32_t buscar_dato_en_memoria(uint32_t dir_fisica) {
  xlog(COLOR_CONEXION, "Buscando en memoria la dir fisica: %d", dir_fisica);

  uint32_t dato_buscado = 0;
  int num_marco_buscado = dir_fisica / tam_marcos;
  int desplazamiento = dir_fisica % tam_marcos;

  bool es_el_marco(t_marco * marco) {
    return (marco->num_marco == num_marco_buscado);
  }

  t_marco* marco = list_find(tabla_marcos, (void*)es_el_marco);
  int inicio = (num_marco_buscado * tam_marcos) + desplazamiento;
  memcpy(&dato_buscado, memoria_principal + inicio, sizeof(int));
  return dato_buscado;
}
