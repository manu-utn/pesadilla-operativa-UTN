#include "memoria.h"
#include "simulacion.h"
#include <stdio.h>

int main() {
  inicializar_estructuras();

  // TODO: comentado temporalmente, pido que se utilicen las macros para las rutas
  /*
  char* path_punto_monataje = config_get_string_value(config, "PATH_SWAP");
  crear_punto_de_montaje(path_punto_monataje);

  void* textoPrueba = "123Probaasasa";
  void* textoPrueba2 = "caca";

  escribir_archivo_swap("/home/utnso/swap/0.swap", textoPrueba, 2, 32);
  escribir_archivo_swap("/home/utnso/swap/0.swap", textoPrueba2, 1, 32);
  eliminar_archivo_swap(1, path_punto_monataje);
  leer_archivo_swap("/home/utnso/swap/0.swap", 2, 32);*/

  pthread_t th1;
  pthread_create(&th1, NULL, (void*)escuchar_conexiones, NULL), pthread_detach(th1);

  inicializar_estructuras_de_este_proceso(0, 500);

  mostrar_tabla_marcos();
  imprimir_tablas_de_paginas();

  // simular_asignacion_marcos_1();
  // simular_asignacion_marcos_2();
  // simular_solicitud_marco_por_mmu();

  pthread_exit(0);
}

// TODO: validar si remover, ya no se está utilizando
void* reservar_memoria_inicial(int size_memoria_total) {
  void* memoria_total = malloc(size_memoria_total);

  memset(memoria_total, 0, size_memoria_total);

  return memoria_total;
}

bool tiene_marco_asignado_entrada_TP(t_entrada_tabla_segundo_nivel* entrada) {
  return entrada->num_marco != -1;
}

int obtener_marco(int numero_tabla_paginas_segundo_nivel, int numero_entrada_TP_segundo_nivel) {
  int marco = 0;
  xlog(COLOR_TAREA, "Buscando un marco disponible... (TP_2do_nivel=%d, numero_entrada=%d)", numero_tabla_paginas_segundo_nivel, numero_entrada_TP_segundo_nivel);

  // TODO: evaluar más en detalle como manejar este error, por el momento retornamos -1
  if (!dictionary_has_key(tablas_de_paginas_segundo_nivel, string_itoa(numero_entrada_TP_segundo_nivel))) {
    return -1;
  } else {
    t_entrada_tabla_segundo_nivel* entrada_segundo_nivel = obtener_entrada_tabla_segundo_nivel(numero_tabla_paginas_segundo_nivel, numero_entrada_TP_segundo_nivel);
    int pid = obtener_pid_asignado_TP_segundo_nivel(numero_entrada_TP_segundo_nivel);

    if (tiene_marco_asignado_entrada_TP(entrada_segundo_nivel)) {
      marco = entrada_segundo_nivel->num_marco;

      xlog(COLOR_TAREA,
           "Se encontró el marco asignado a la entrada solicitada (TP_2do_nivel=%d, numero_entrada=%d, pid=%d, "
           "numero_marco=%d)",
           numero_tabla_paginas_segundo_nivel,
           numero_entrada_TP_segundo_nivel,
           pid,
           marco);
    } else if (hay_marcos_libres_asignados_al_proceso(pid)) {
      xlog(COLOR_TAREA,
           "Buscando alguno de los marcos libre de los asignados al proceso... (pid=%d, cantidad_marcos_disponibles=%d)",
           pid,
           cantidad_marcos_libres_asignados_al_proceso(pid));

      marco = obtener_y_asignar_primer_marco_libre_asignado_al_proceso(pid, entrada_segundo_nivel);

      xlog(COLOR_TAREA,
           "Se obtuvo el primer marco libre para a la entrada solicitada (TP_2do_nivel=%d, numero_entrada=%d, "
           "numero_marco=%d)",
           numero_tabla_paginas_segundo_nivel,
           numero_entrada_TP_segundo_nivel,
           marco);

      // según los algoritmos de reemplazo (clock/clock modificado): si bit_de_uso == 0 && bit_de_presencia == 1,
      // entonces bit_de_uso=1 (se habilita de nuevo) podríamos siempre setearlo a 1, pero agregamos éste condicional
      // para recordar/relacionar con la teoría
      if (entrada_segundo_nivel->bit_uso == 0)
        entrada_segundo_nivel->bit_uso = 1;
    } else {
      // si no tiene marcos libres => ejecutar algoritmo de sustitución de páginas
      // marco = obtener_y_asignar_marco_segun_algoritmo_de_reemplazo(pid, entrada_segundo_nivel);
      marco = obtener_y_asignar_marco_segun_algoritmo_de_reemplazo(pid, numero_tabla_paginas_segundo_nivel, entrada_segundo_nivel);

      xlog(COLOR_TAREA,
           "Se aplicó algoritmo de reemplazo y se obtuvo un marco para a la entrada solicitada (algoritmo=%s, "
           "TP_2do_nivel=%d, "
           "numero_entrada=%d, "
           "numero_marco=%d)",
           obtener_algoritmo_reemplazo_por_config(),
           numero_tabla_paginas_segundo_nivel,
           numero_entrada_TP_segundo_nivel,
           marco);
    }
  }

  return marco;
}

// TODO: lógica repetida con obtener_primer_marco_libre y hay_marcos_libres_asignados_al_proceso
int cantidad_marcos_libres_asignados_al_proceso(int pid) {
  bool marco_libre_asignado_a_este_proceso(t_marco * marco) {
    return marco->pid == pid && marco->ocupado == 0;
  }

  return list_count_satisfying(tabla_marcos, (void*)marco_libre_asignado_a_este_proceso);
}

// TODO: lógica repetida con obtener_primer_marco_libre
bool hay_marcos_libres_asignados_al_proceso(int pid) {
  bool marco_libre_asignado_a_este_proceso(t_marco * marco) {
    return marco->pid == pid && marco->ocupado == 0;
  }

  return list_any_satisfy(tabla_marcos, (void*)marco_libre_asignado_a_este_proceso);
}

// TODO: validar lógica repetida
t_tabla_segundo_nivel* obtener_TP_segundo_nivel(int numero_TP_primer_nivel, int numero_entrada_TP_primer_nivel) {
  t_tabla_primer_nivel* TP_primer_nivel = dictionary_get(tablas_de_paginas_primer_nivel, string_itoa(numero_TP_primer_nivel));
  t_entrada_tabla_primer_nivel* entrada_primer_nivel = dictionary_get(TP_primer_nivel->entradas_primer_nivel, string_itoa(numero_entrada_TP_primer_nivel));

  t_tabla_segundo_nivel* TP_segundo_nivel = dictionary_get(tablas_de_paginas_segundo_nivel, string_itoa(entrada_primer_nivel->num_tabla_segundo_nivel));

  return TP_segundo_nivel;
}

// TODO: validar lógica repetida
int obtener_numero_TP_segundo_nivel(int numero_TP_primer_nivel, int numero_entrada_TP_primer_nivel) {
  t_tabla_primer_nivel* TP_primer_nivel = dictionary_get(tablas_de_paginas_primer_nivel, string_itoa(numero_TP_primer_nivel));
  t_entrada_tabla_primer_nivel* entrada_primer_nivel = dictionary_get(TP_primer_nivel->entradas_primer_nivel, string_itoa(numero_entrada_TP_primer_nivel));

  return entrada_primer_nivel->num_tabla_segundo_nivel;
}

void dividir_memoria_principal_en_marcos() {
  xlog(COLOR_TAREA,
       "Dividiendo la memoria en marcos (memoria_bytes=%d, tamanio_pagina_bytes=%d, cantidad_marcos_en_memoria=%d)",
       obtener_tamanio_memoria_por_config(),
       obtener_tamanio_pagina_por_config(),
       obtener_cantidad_marcos_en_memoria());

  for (int numero_marco = 0; numero_marco < obtener_cantidad_marcos_en_memoria(); numero_marco++) {
    t_marco* marco = malloc(sizeof(t_marco));
    marco->num_marco = numero_marco;
    marco->direccion = 0;
    marco->pid = -1; // porque no tiene un proceso asignado al principio

    // para simular un bitmap de marcos libres/ocupados
    marco->ocupado = 0;

    // para facilitar el algoritmo de reemplazo
    marco->apuntado_por_puntero_de_clock = false;
    marco->entrada_segundo_nivel = NULL;

    list_add(tabla_marcos, marco);
  }
}

int obtener_cantidad_marcos_en_memoria() {
  return obtener_tamanio_memoria_por_config() / obtener_tamanio_pagina_por_config();
}

// comentado temporalmente, un merge de swap generò conflictos en memoria
// void inicializar_estructuras_de_este_proceso(uint32_t pid, int tam_proceso)
int inicializar_estructuras_de_este_proceso(int pid, int tam_proceso) {
  xlog(COLOR_TAREA, "Inicializando estructuras en memoria para un proceso (pid=%d, tamanio_bytes=%d)", pid, tam_proceso);

  t_tabla_primer_nivel* tabla_primer_nivel = tabla_paginas_primer_nivel_create(pid);
  int numero_tabla_primer_nivel = tabla_primer_nivel->num_tabla;

  inicializar_archivo_swap(pid, tam_proceso);

  // agregamos una TP_primer_nivel en una estructura global
  dictionary_put(tablas_de_paginas_primer_nivel, string_itoa(tabla_primer_nivel->num_tabla), tabla_primer_nivel);

  xlog(COLOR_TAREA,
       "TP de primer nivel agregada a una estructura global (numero_TP=%d, cantidad_TP_primer_nivel=%d)",
       tabla_primer_nivel->num_tabla,
       dictionary_size(tablas_de_paginas_primer_nivel));

  return numero_tabla_primer_nivel;
}

void liberar_estructuras_en_memoria_de_este_proceso(int pid) {
  t_tabla_primer_nivel* TP_primer_nivel = obtener_tabla_paginas_primer_nivel_por_pid(pid);
  int numero_tabla_primer_nivel = TP_primer_nivel->num_tabla;

  dictionary_remove_and_destroy(tablas_de_paginas_primer_nivel, string_itoa(numero_tabla_primer_nivel), (void*)tabla_paginas_primer_nivel_destroy);
}

void tabla_paginas_primer_nivel_destroy(t_tabla_primer_nivel* tabla_paginas_primer_nivel) {
  dictionary_destroy_and_destroy_elements(tabla_paginas_primer_nivel->entradas_primer_nivel, (void*)entrada_primer_nivel_destroy);

  xlog(COLOR_RECURSOS, "Se liberaron con éxito los recursos asignados a la tabla de primer nivel (tp_primer_nivel=%d)", tabla_paginas_primer_nivel->num_tabla);

  free(tabla_paginas_primer_nivel);
}

void entrada_primer_nivel_destroy(t_entrada_tabla_primer_nivel* entrada_primer_nivel) {
  int numero_tabla_primer_nivel = entrada_primer_nivel->num_tabla_primer_nivel;
  int numero_entrada_primer_nivel = entrada_primer_nivel->entrada_primer_nivel;

  t_tabla_segundo_nivel* TP_segundo_nivel = obtener_TP_segundo_nivel(numero_tabla_primer_nivel, numero_entrada_primer_nivel);

  dictionary_destroy_and_destroy_elements(TP_segundo_nivel->entradas_segundo_nivel, (void*)entrada_segundo_nivel_destroy);

  xlog(COLOR_RECURSOS,
       "Se liberaron con éxito los recursos asignados a la entrada de primer nivel (tp_primer_nivel=%d, entrada=%d)",
       entrada_primer_nivel->num_tabla_primer_nivel,
       entrada_primer_nivel->entrada_primer_nivel);

  free(entrada_primer_nivel);
}

void entrada_segundo_nivel_destroy(t_entrada_tabla_segundo_nivel* entrada_segundo_nivel) {
  xlog(COLOR_RECURSOS,
       "Se liberaron con éxito los recursos asignados a la entrada de segundo nivel (tp_segundo_nivel=%d, entrada=%d)",
       entrada_segundo_nivel->numero_tabla_segundo_nivel,
       entrada_segundo_nivel->entrada_segundo_nivel);

  free(entrada_segundo_nivel);
}

t_entrada_tabla_segundo_nivel* obtener_entrada_tabla_segundo_nivel(int numero_TP_segundo_nivel, int numero_entrada_TP_segundo_nivel) {
  t_tabla_segundo_nivel* TP_segundo_nivel = dictionary_get(tablas_de_paginas_segundo_nivel, string_itoa(numero_TP_segundo_nivel));
  t_entrada_tabla_segundo_nivel* entrada_segundo_nivel = dictionary_get(TP_segundo_nivel->entradas_segundo_nivel, string_itoa(numero_entrada_TP_segundo_nivel));

  return entrada_segundo_nivel;
}

int obtener_pid_asignado_TP_segundo_nivel(int numero_entrada_TP_segundo_nivel) {
  t_tabla_segundo_nivel* TP_segundo_nivel = dictionary_get(tablas_de_paginas_segundo_nivel, string_itoa(numero_entrada_TP_segundo_nivel));

  return TP_segundo_nivel->pid;
}

// TODO: validar
// TODO: lógica repetida con hay_marcos_disponibles_asignados_al_proceso
int obtener_y_asignar_primer_marco_libre_asignado_al_proceso(int pid, t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel) {
  int marco_libre_asignado_a_este_proceso(t_marco * marco) {
    return marco->pid == pid && marco->ocupado == 0;
  }

  t_marco* marco_libre = list_find(tabla_marcos, (void*)marco_libre_asignado_a_este_proceso);
  marco_libre->ocupado = 1;

  entrada_TP_segundo_nivel->num_marco = marco_libre->num_marco;

  // para facilitar el algoritmo de reemplazo
  marco_libre->entrada_segundo_nivel = entrada_TP_segundo_nivel;

  return marco_libre->num_marco;
}


// llena el espacio en memoria con ceros
void llenar_memoria_mock() {
  int marco = 0, offset = 0;

  xlog(COLOR_TAREA, "Llenando los marcos de memoria con ceros..")

    while (marco < obtener_cantidad_marcos_en_memoria()) {
    memset(memoria_principal + offset, 0, obtener_tamanio_pagina_por_config());
    printf("marco=%d, %p + %d = %p", marco, memoria_principal, offset, memoria_principal + offset);

    char* datos_marco = mem_hexstring(memoria_principal + offset, obtener_tamanio_pagina_por_config());
    printf("%s\n\n", datos_marco);

    marco++;
    offset = offset + obtener_tamanio_pagina_por_config();
  }

  printf("\n");
}

t_tabla_primer_nivel* obtener_tabla_paginas_primer_nivel_por_pid(int pid) {
  t_tabla_primer_nivel* tabla_paginas_primer_nivel = malloc(sizeof(t_tabla_primer_nivel));

  for (int cantidad_tablas_paginas_primer_nivel_leidas = 0; cantidad_tablas_paginas_primer_nivel_leidas < cantidad_tablas_paginas_primer_nivel();
       cantidad_tablas_paginas_primer_nivel_leidas++) {
    tabla_paginas_primer_nivel = dictionary_get(tablas_de_paginas_primer_nivel, string_itoa(cantidad_tablas_paginas_primer_nivel_leidas));

    if (tabla_paginas_primer_nivel->pid == pid)
      break;
  }

  return tabla_paginas_primer_nivel;
}

t_list* obtener_marcos_asignados_a_este_proceso(int pid) {
  bool marco_libre_asignado_a_este_proceso(t_marco * marco) {
    return marco->pid == pid;
  }

  bool marco_menor_numero(t_marco * marco_menor_numero, t_marco * marco_mayor_numero) {
    return marco_menor_numero->num_marco <= marco_mayor_numero->num_marco;
  }

  t_list* marcos_asignados = list_filter(tabla_marcos, (void*)marco_libre_asignado_a_este_proceso);

  // necesario mantener siempre el mismo orden, para mover el puntero del algoritmo de reemplazo en la cola circular
  t_list* marcos_asignados_ordenados_menor_a_mayor_numero = list_sorted(marcos_asignados, (void*)marco_menor_numero);

  return marcos_asignados_ordenados_menor_a_mayor_numero;
}

int cantidad_tablas_paginas_primer_nivel() {
  return dictionary_size(tablas_de_paginas_primer_nivel);
}

t_tabla_primer_nivel* tabla_paginas_primer_nivel_create(int pid) {
  t_tabla_primer_nivel* tabla_paginas_primer_nivel = malloc(sizeof(t_tabla_primer_nivel));

  // TODO: validar si conviene usar otra manera
  // contamos la cantidad de elementos en la estructura global (en el diccionario) y le sumamos uno
  int numero_tabla_primer_nivel = cantidad_tablas_paginas_primer_nivel() + 1;

  xlog(COLOR_TAREA, "Creando TP de primer nivel... (pid=%d, numero=%d)", pid, numero_tabla_primer_nivel);
  tabla_paginas_primer_nivel->num_tabla = numero_tabla_primer_nivel;

  // requerido para liberar estructuras en memoria, cuando un proceso finalizar
  tabla_paginas_primer_nivel->pid = pid;

  tabla_paginas_primer_nivel->entradas_primer_nivel = dictionary_create();

  for (int numero_entrada_primer_nivel = 0; numero_entrada_primer_nivel < obtener_cantidad_entradas_por_tabla_por_config(); numero_entrada_primer_nivel++) {
    t_entrada_tabla_primer_nivel* entrada_primer_nivel = malloc(sizeof(t_entrada_tabla_primer_nivel));

    // esto identifica cada entrada de TP 1er nivel, la MMU accede a ésta usando
    // floor(numero_pagina_DL/cant_entradas_por_tabla)
    entrada_primer_nivel->entrada_primer_nivel = numero_entrada_primer_nivel;
    entrada_primer_nivel->num_tabla_primer_nivel = numero_tabla_primer_nivel;

    // TODO: validar si se debe usar otro criterio para el numero_tabla_segundo_nivel
    t_tabla_segundo_nivel* tabla_paginas_segundo_nivel = tabla_paginas_segundo_nivel_create(numero_entrada_primer_nivel, 1);

    // agregamos una TP_segundo_nivel en una estructura global
    dictionary_put(tablas_de_paginas_segundo_nivel, string_itoa(tabla_paginas_segundo_nivel->num_tabla), tabla_paginas_segundo_nivel);
    xlog(COLOR_TAREA,
         "TP de segundo nivel agregada a una estructura global (numero_TP=%d, cantidad_TP_segundo_nivel=%d)",
         tabla_paginas_segundo_nivel->num_tabla,
         dictionary_size(tablas_de_paginas_segundo_nivel));


    entrada_primer_nivel->num_tabla_segundo_nivel = tabla_paginas_segundo_nivel->num_tabla;

    // agregamos una entrada_primer_nivel a la TP_primer_nivel
    dictionary_put(tabla_paginas_primer_nivel->entradas_primer_nivel, string_itoa(entrada_primer_nivel->entrada_primer_nivel), entrada_primer_nivel);
  }

  xlog(COLOR_TAREA,
       "TP de primer nivel creada con éxito (TP_1er_nivel_numero=%d, cantidad_entradas=%d)",
       tabla_paginas_primer_nivel->num_tabla,
       dictionary_size(tabla_paginas_primer_nivel->entradas_primer_nivel));

  return tabla_paginas_primer_nivel;
}

void inicializar_entrada_de_tabla_paginas(t_entrada_tabla_segundo_nivel* entrada_tabla_segundo_nivel) {
  // TODO: evaluar si siempre corresponde que se inicialize en 1, al menos para el clock y clock-m supongo que si
  entrada_tabla_segundo_nivel->bit_uso = 1;

  entrada_tabla_segundo_nivel->bit_modif = 0;

  // TODO: evaluar si corresponde que esté inicializado en 0
  entrada_tabla_segundo_nivel->bit_presencia = 0;

  entrada_tabla_segundo_nivel->num_marco = -1; // valor negativo porque no tiene un marco asignado
}

t_tabla_segundo_nivel* tabla_paginas_segundo_nivel_create(int numero_tabla_segundo_nivel, int pid) {
  t_tabla_segundo_nivel* tabla_paginas_segundo_nivel = malloc(sizeof(t_tabla_segundo_nivel));

  xlog(COLOR_TAREA, "Creando TP de segundo nivel... (numero_TP=%d)", numero_tabla_segundo_nivel);

  // TODO: esto debe coincidir con num_tabla_segundo_nivel que tiene la entrada de la TP de primer nivel
  tabla_paginas_segundo_nivel->num_tabla = numero_tabla_segundo_nivel;
  tabla_paginas_segundo_nivel->pid = pid;
  tabla_paginas_segundo_nivel->entradas_segundo_nivel = dictionary_create();

  for (int numero_entrada_segundo_nivel = 0; numero_entrada_segundo_nivel < obtener_cantidad_entradas_por_tabla_por_config(); numero_entrada_segundo_nivel++) {
    t_entrada_tabla_segundo_nivel* entrada_tabla_segundo_nivel = malloc(sizeof(t_entrada_tabla_segundo_nivel));
    entrada_tabla_segundo_nivel->entrada_segundo_nivel = numero_entrada_segundo_nivel;

    // necesario como metadata para los marcos, algoritmos de sustitución..
    entrada_tabla_segundo_nivel->numero_tabla_segundo_nivel = numero_tabla_segundo_nivel;

    inicializar_entrada_de_tabla_paginas(entrada_tabla_segundo_nivel);

    dictionary_put(tabla_paginas_segundo_nivel->entradas_segundo_nivel, string_itoa(entrada_tabla_segundo_nivel->entrada_segundo_nivel), entrada_tabla_segundo_nivel);
  }

  xlog(COLOR_TAREA,
       "TP de segundo nivel creada con éxito (numero_TP=%d, cantidad_entradas=%d)",
       tabla_paginas_segundo_nivel->num_tabla,
       dictionary_size(tabla_paginas_segundo_nivel->entradas_segundo_nivel));

  return tabla_paginas_segundo_nivel;
}

int obtener_posicion_de_marco_del_listado(t_marco* marco, t_list* lista_marcos) {
  for (int posicion = 0; posicion < list_size(lista_marcos); posicion++) {
    if (marco == (t_marco*)list_get(lista_marcos, posicion)) {
      return posicion;
    }
  }

  return -1;
}

t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel_create(int num_entrada, int num_marco, int bit_uso, int bit_modif, int bit_presencia) {
  t_entrada_tabla_segundo_nivel* entrada = malloc(sizeof(t_entrada_tabla_segundo_nivel));

  entrada->entrada_segundo_nivel = num_entrada;
  entrada->num_marco = num_marco;
  entrada->bit_uso = bit_uso;
  entrada->bit_modif = bit_modif;
  entrada->bit_presencia = bit_presencia;

  return entrada;
}

t_marco* marco_create(int numero, int pid, t_estado_marco estado) {
  t_marco* marco = malloc(sizeof(t_marco));
  marco->pid = pid;
  marco->num_marco = numero;
  marco->ocupado = estado;

  return marco;
}

t_marco* obtener_marco_de_memoria(int numero_marco) {
  int es_este_marco(t_marco * marco) {
    return marco->num_marco == numero_marco;
  }

  t_marco* marco = list_find(tabla_marcos, (void*)es_este_marco);

  return marco;
}

// TODO: evaluar, que ocurre en el algoritmo de sustiticón..
// - y la anterior entrada que tenia asignada este marco?
// - dos o más entradas comparten el mismo marco pero éste marco ya no apunta a esa entrada..
void reasignar_marco(int numero_marco, int pid, t_entrada_tabla_segundo_nivel* entrada_TP_segundo_nivel) {
  t_marco* marco = obtener_marco_de_memoria(numero_marco);

  marco->pid = pid;
  marco->ocupado = 1;

  entrada_TP_segundo_nivel->num_marco = numero_marco;

  // para facilitar el algoritmo de reemplazo
  marco->entrada_segundo_nivel = entrada_TP_segundo_nivel;
}

void algoritmo_clock_puntero_apuntar_al_marco(int numero_marco) {
  t_marco* marco = obtener_marco_de_memoria(numero_marco);
  marco->apuntado_por_puntero_de_clock = true;
}


int reemplazar_entrada_en_marco_de_memoria(t_entrada_tabla_segundo_nivel* entrada_victima, t_entrada_tabla_segundo_nivel* nueva_entrada) {
  int numero_marco = entrada_victima->num_marco;
  t_marco* marco = obtener_marco_de_memoria(numero_marco);

  entrada_victima->num_marco = -1;
  nueva_entrada->num_marco = numero_marco;

  marco->entrada_segundo_nivel = nueva_entrada;
  marco->numero_tabla_segundo_nivel = nueva_entrada->numero_tabla_segundo_nivel;

  return numero_marco;
}

void inicializar_estructuras() {
  estado_conexion_memoria = true;
  logger = iniciar_logger(DIR_LOG_MESSAGES, "MEMORIA");
  config = iniciar_config(DIR_MEMORIA_CFG);

  uint32_t size_memoria = config_get_int_value(config, "TAM_MEMORIA");
  tamanio_marco = config_get_int_value(config, "TAM_PAGINA");

  memoria_principal = malloc(size_memoria);

  ULTIMO_ID_1er_nivel = 0;
  ULTIMO_ID_2do_nivel = 0;

  llenar_memoria_mock();

  tablas_de_paginas_primer_nivel = dictionary_create();
  tablas_de_paginas_segundo_nivel = dictionary_create();
  tabla_marcos = list_create();

  dividir_memoria_principal_en_marcos();
}
