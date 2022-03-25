#include <stdio.h>
#include "sample.h"
#include <lib_mylib.h> // <-- STATIC LIB

int main()
{
  // función de la biblioteca static
  saludar();

  return 0;
}
