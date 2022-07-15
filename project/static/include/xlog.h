#ifndef UTILS_XLOG_H_
#define UTILS_XLOG_H_

#include <commons/log.h>

#define COLOR_RESET "\x1b[0m"
#define COLOR_ERROR "\x1b[31m" // ROJO
#define COLOR_PAQUETE "\x1b[32m" "[PAQUETE] " // VERDE
#define COLOR_TAREA "\x1b[33m" "[TAREA] " // AMARILLO
#define COLOR_CONEXION "\x1b[34m" "[CONEXION] " // AZUL
#define COLOR_RECURSOS "\x1b[35m" "[RECURSOS] " // MAGENTA
#define COLOR_SERIALIZADO "\x1b[36m" "[SERIALIZADO] " // CYAN
#define COLOR_DESERIALIZADO "\x1b[36m" "[DESERIALIZADO] " // CYAN
#define COLOR_INFO "\x1b[37m" "[INFO] " // BLANCO

int SERIALIZADO_ATRIBUTOS;
int DESERIALIZADO_ATRIBUTOS;

void serializado_reset();
void deserializado_reset();

#define xlog(color, texto, args...) log_info(logger, color texto COLOR_RESET, ##args);

#define serializado_log(texto, args...)                             \
  log_info(logger, COLOR_SERIALIZADO ">  " texto COLOR_RESET, ##args); \
  SERIALIZADO_ATRIBUTOS++;

#define serializado_inicio(estructura) \
  log_info(logger, COLOR_SERIALIZADO "estructura: " estructura COLOR_RESET); \
  serializado_reset();

#define serializado_fin(cantidad_atributos) \
  log_info(logger, COLOR_SERIALIZADO "atributos serializados: %d" COLOR_RESET, SERIALIZADO_ATRIBUTOS);


#define deserializado_log(texto, args...) \
  log_info(logger, COLOR_DESERIALIZADO ">  " texto COLOR_RESET, ##args); \
  DESERIALIZADO_ATRIBUTOS++;

#define deserializado_inicio(estructura) \
  log_info(logger, COLOR_DESERIALIZADO "estructura: " estructura COLOR_RESET); \
  deserializado_reset();

#define deserializado_fin(cantidad_atributos) \
  log_info(logger, COLOR_DESERIALIZADO "atributos deserializados: %d" COLOR_RESET, DESERIALIZADO_ATRIBUTOS);

t_log *logger;
#endif
