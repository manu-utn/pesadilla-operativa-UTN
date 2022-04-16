#include "sample.h"
#include "cpu.h"
#include <commons/config.h>
#include <libstatic.h> // <-- STATIC LIB
#include <stdio.h>
int main() {
  // función de la biblioteca static
  // saludar();
  cargarConfiguracion();

  iniciar_ciclo_instruccion();


  limpiarConfiguracion();
}
