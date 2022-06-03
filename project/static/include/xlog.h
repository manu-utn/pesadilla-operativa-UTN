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

#define xlog(color, texto, args...) log_info(logger, color texto COLOR_RESET, ##args);

t_log *logger;
#endif
