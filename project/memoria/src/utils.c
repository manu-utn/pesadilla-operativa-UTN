#include "utils.h"
#include "libstatic.h"
#include "memoria.h"

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


  t_tabla_segundo_nivel* tabla = malloc(sizeof(t_tabla_segundo_nivel));
  t_list* entradas = list_create();

  t_entrada_tabla_segundo_nivel* entrada1 = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  t_entrada_tabla_segundo_nivel* entrada2 = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  t_entrada_tabla_segundo_nivel* entrada3 = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  t_entrada_tabla_segundo_nivel* entrada4 = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  t_entrada_tabla_segundo_nivel* entrada5 = malloc(sizeof(t_entrada_tabla_segundo_nivel));

  entrada1->entrada_segundo_nivel = 1;
  entrada1->num_marco = 3;
  entrada1->bit_uso = 1;
  entrada1->bit_modif = 0;
  entrada1->bit_presencia = 1;

  entrada2->entrada_segundo_nivel = 2;
  entrada2->num_marco = 1;
  entrada2->bit_uso = 1;
  entrada2->bit_modif = 0;
  entrada2->bit_presencia = 1;

  entrada3->entrada_segundo_nivel = 3;
  entrada3->num_marco = 2;
  entrada3->bit_uso = 1;
  entrada3->bit_modif = 0;
  entrada3->bit_presencia = 1;

  entrada4->entrada_segundo_nivel = 4;
  entrada4->num_marco = NULL;
  entrada4->bit_uso = 0;
  entrada4->bit_modif = 0;
  entrada4->bit_presencia = 0;


  entrada5->entrada_segundo_nivel = 5;
  entrada5->num_marco = NULL;
  entrada5->bit_uso = 0;
  entrada5->bit_modif = 0;
  entrada5->bit_presencia = 0;

  list_add(entradas, entrada1);
  list_add(entradas, entrada2);
  list_add(entradas, entrada3);
  list_add(entradas, entrada4);
  list_add(entradas, entrada5);

  tabla->entradas = entradas;

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


void inicializar_estructuras() {
  estado_conexion_memoria = true;
  logger = iniciar_logger(DIR_LOG_MESSAGES, "MEMORIA");
  config = iniciar_config(DIR_MEMORIA_CFG);
  algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
  cant_marcos_por_proceso = config_get_int_value(config, "MARCOS_POR_PROCESO");
  uint32_t size_memoria = config_get_int_value(config, "TAM_MEMORIA");
  memoria_principal = reservar_memoria_inicial(size_memoria);
  size_memoria_principal = config_get_int_value(config, "TAM_MEMORIA");
  puntero_clock = 0;
  llenar_memoria_mock();
  tam_marcos = config_get_int_value(config, "TAM_PAGINA");
  diccionario_tablas = dictionary_create();
  tabla_marcos = list_create();
  lista_tablas_segundo_nivel = list_create();
  xlog(COLOR_CONEXION, "Tamanio tabla de marcos: %d:", inicializar_tabla_marcos());
  reservar_marcos_mock();

  // mostrar_tabla_marcos();
  // mem_hexdump(memoria_principal, size_memoria_principal);
  inicializar_proceso(0, 4, 500);
}