typedef enum {MENSAJE_IMPORTANTE, MENSAJE_COMUN} TIPO_MENSAJE;

typedef struct {
  char* texto;
  TIPO_MENSAJE tipo;
} Mensaje;

void imprimirMensaje(Mensaje);
Mensaje crearMensajeImportante(char *);
