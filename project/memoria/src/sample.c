#include "sample.h"
#include "memoria.h"
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>
#include <utils.h>

int main() {
  // función de la biblioteca static
  inicializar_estructuras();
  pthread_t th;
  pthread_create(&th, NULL, escuchar_conexiones, NULL), pthread_detach(th);

  /* PARA TESTEAR ALGORITMOS DE REEMPLAZO
   reservar_marcos_mock();

    t_entrada_tabla_segundo_nivel* entrada = malloc(sizeof(t_entrada_tabla_segundo_nivel));

    entrada->entrada_segundo_nivel = 5;
    entrada->num_marco = NULL;
    entrada->bit_uso = 0;
    entrada->bit_modif = 0;
    entrada->bit_presencia = 0;

    int marco_victima = ejecutar_clock(marcos_prueba_clock, entrada);
  */

  pthread_exit(0);
}
