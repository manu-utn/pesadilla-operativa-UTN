#include "utils.h"
#include "libstatic.h"
#include "memoria.h"
#include <commons/collections/dictionary.h>

void inicializar_estructuras() {
  estado_conexion_memoria = true;
  logger = iniciar_logger(DIR_LOG_MESSAGES, "MEMORIA");
  config = iniciar_config(DIR_MEMORIA_CFG);

  // TODO: validar porque aparece dos veces
  uint32_t size_memoria = config_get_int_value(config, "TAM_MEMORIA");

  memoria_principal = malloc(obtener_tamanio_memoria_por_config());

  // TODO: validar si deprecar, la nueva implementación no lo está utilizando
  puntero_clock = 0;

  // TODO: validar
  llenar_memoria_mock();

  tablas_de_paginas_primer_nivel = dictionary_create();
  tablas_de_paginas_segundo_nivel = dictionary_create();
  tabla_marcos = list_create();

  dividir_memoria_principal_en_marcos();

  // TODO: validar
  // reservar_marcos_mock();

  // lo estamos usando en sample.c
  // mostrar_tabla_marcos();
  // mem_hexdump(memoria_principal, size_memoria_principal);

  // lo estamos usando en sample.c
  // inicializar_proceso(0, 4, 500);
}

// lo comentamos arriba para probar la otra versión en sample.c
// intentámos adaptarlo el mock a la versión con las otras estructuras
void reservar_marcos_mock() {
  t_marco* marco1 = list_get(tabla_marcos, 0);
  t_marco* marco2 = list_get(tabla_marcos, 1);
  t_marco* marco3 = list_get(tabla_marcos, 2);
  marcos_prueba_clock = list_create();

  marco1->ocupado = 1;
  marco1->pid = 1;
  list_add_in_index(tabla_marcos, 0, marco1);

  marco2->ocupado = 1;
  marco2->pid = 1;
  list_add_in_index(tabla_marcos, 1, marco2);

  marco3->ocupado = 1;
  marco3->pid = 1;
  list_add_in_index(tabla_marcos, 2, marco3);

  t_entrada_tabla_segundo_nivel* entrada1 = entrada_TP_segundo_nivel_create(0, 3, 1, 0, 1);
  t_entrada_tabla_segundo_nivel* entrada2 = entrada_TP_segundo_nivel_create(1, 1, 1, 0, 1);
  t_entrada_tabla_segundo_nivel* entrada3 = entrada_TP_segundo_nivel_create(2, 2, 1, 0, 1);
  t_entrada_tabla_segundo_nivel* entrada4 = entrada_TP_segundo_nivel_create(3, -1, 0, 0, 0);
  t_entrada_tabla_segundo_nivel* entrada5 = entrada_TP_segundo_nivel_create(4, -1, 0, 0, 0);

  t_tabla_segundo_nivel* tabla_paginas_segundo_nivel = tabla_paginas_segundo_nivel_create(1);
  dictionary_put(
    tabla_paginas_segundo_nivel->entradas_segundo_nivel, string_itoa(entrada1->entrada_segundo_nivel), entrada1);
  dictionary_put(
    tabla_paginas_segundo_nivel->entradas_segundo_nivel, string_itoa(entrada2->entrada_segundo_nivel), entrada2);
  dictionary_put(
    tabla_paginas_segundo_nivel->entradas_segundo_nivel, string_itoa(entrada3->entrada_segundo_nivel), entrada3);
  dictionary_put(
    tabla_paginas_segundo_nivel->entradas_segundo_nivel, string_itoa(entrada4->entrada_segundo_nivel), entrada4);
  dictionary_put(
    tabla_paginas_segundo_nivel->entradas_segundo_nivel, string_itoa(entrada5->entrada_segundo_nivel), entrada5);

  // TODO: (???)
  t_marco_asignado* marco_involucrado1 = malloc(sizeof(t_marco_asignado));
  marco_involucrado1->marco = marco1->num_marco;
  marco_involucrado1->entrada = entrada1;

  t_marco_asignado* marco_involucrado2 = malloc(sizeof(t_marco_asignado));
  marco_involucrado2->marco = marco2->num_marco;
  marco_involucrado2->entrada = entrada2;

  t_marco_asignado* marco_involucrado3 = malloc(sizeof(t_marco_asignado));
  marco_involucrado3->marco = marco3->num_marco;
  marco_involucrado3->entrada = entrada3;

  list_add(marcos_prueba_clock, marco_involucrado1);
  list_add(marcos_prueba_clock, marco_involucrado2);
  list_add(marcos_prueba_clock, marco_involucrado3);
}
