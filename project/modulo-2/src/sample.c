#include "sample.h"
#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  t_list *publicaciones;
  char temp_file[] =
    "logs/publicaciones.txt"; // se creará en la raíz del proyecto
  t_log *logger = log_create(temp_file, "TEST", true, LOG_LEVEL_INFO);
  publicaciones = list_create();

  list_add(publicaciones, crear_publicacion("holis"));

  char *_get_texto(Publicacion * publicacion)
  {
    return publicacion->texto;
  }; // fundamental ese ; en las nested functions

  t_list *textos = list_map(publicaciones, (void *)_get_texto);

  for (int i = 0; i < list_size(textos); i++)
  {
    log_info(logger, "%s", (char *)list_get(textos, i));
  }

  list_destroy_and_destroy_elements(publicaciones, (void *)publicacion_destroy);
  log_destroy(logger);

  return 0;
}

Publicacion *crear_publicacion(char *texto)
{
  Publicacion *publicacion = malloc(sizeof(Publicacion));

  publicacion->texto = strdup(texto);

  return publicacion;
}

void publicacion_destroy(Publicacion *publicacion)
{
  free(publicacion->texto);
  free(publicacion);
}

void imprimir_publicacion(Publicacion publicacion)
{
  printf("%s\n", publicacion.texto);
}
