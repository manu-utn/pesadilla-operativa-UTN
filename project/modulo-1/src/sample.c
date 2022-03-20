#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sample.h"
#include <commons/collections/list.h>
#include <commons/log.h>
#include "sample1.h"
#include "sample2.h"

int main(){
  int numero;
  numero=5;
  numero++;
  numero++;
  numero=0;

  int resultado = sumar(1,1);
  printf("1+1 es %d\n", resultado);

  t_list *mensajes;
  // es necesario la ruta absoluta para que lo entienda el debugger
  char temp_file_path[] = "/home/jelou/Documents/git/manu-cproject/project/modulo-1/";
  //char temp_file_path[] = "/home/utnso/tp/project/modulo-1/";

  char temp_file[] = "logs/sample.txt"; // se creará en la raíz del proyecto
  strcat(temp_file_path, temp_file);

  t_log *logger = log_create(temp_file_path, "TEST", true, LOG_LEVEL_INFO);
  mensajes = list_create();

  list_add(mensajes, crear_mensaje_importante("holis"));
  list_add(mensajes, crear_mensaje_importante("que tal?"));

  char *_get_texto(Mensaje* mensaje) { return mensaje->texto; }

  t_list *textos = list_map(mensajes, (void *)_get_texto);

  for (int i = 0; i < list_size(textos); i++) {
    log_info(logger, "%s", (char*) list_get(textos, i));
  }

  list_destroy_and_destroy_elements(mensajes, (void *)mensaje_destroy);
  list_destroy(textos);
  log_destroy(logger);

  return 0;
}

Mensaje* crear_mensaje_importante(char *texto) {
  Mensaje* mensaje = malloc(sizeof(Mensaje));

  mensaje->texto = strdup(texto);
  mensaje->tipo = MENSAJE_IMPORTANTE;

  return mensaje;
}

void mensaje_destroy(Mensaje* mensaje) {
  free(mensaje->texto);
  free(mensaje);
}

void imprimir_mensaje(Mensaje mensaje) {
  if (mensaje.tipo == MENSAJE_IMPORTANTE) {
    printf("ALERTA!! ");
  }

  printf("%s\n", mensaje.texto);
}
