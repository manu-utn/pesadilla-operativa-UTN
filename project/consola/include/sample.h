#ifndef __SAMPLE__H
#define __SAMPLE__H
typedef struct {
  char* texto;
} Publicacion;

void imprimir_publicacion(Publicacion);
Publicacion* crear_publicacion(char *);
void publicacion_destroy(Publicacion*);
#endif
