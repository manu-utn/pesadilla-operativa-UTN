#include <stdio.h>
#include "sample.h"

int main(){
  Mensaje m = crearMensajeImportante("hola mundo cruel..! T_T");

  imprimirMensaje(m);

  return 0;
}

Mensaje crearMensajeImportante(char *texto) {
  Mensaje mensaje = {texto, MENSAJE_IMPORTANTE};

  return mensaje;
}

void imprimirMensaje(Mensaje mensaje) {
  if (mensaje.tipo == MENSAJE_IMPORTANTE) {
    printf("ALERTA!! ");
  }

  printf("%s\n", mensaje.texto);
}
