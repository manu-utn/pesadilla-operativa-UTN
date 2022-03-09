typedef enum {MENSAJE_IMPORTANTE, MENSAJE_COMUN} TIPO_MENSAJE;

typedef struct {
  char* texto;
  TIPO_MENSAJE tipo;
} Mensaje;

void imprimir_mensaje(Mensaje);
Mensaje* crear_mensaje_importante(char *);
void mensaje_destroy(Mensaje*);
