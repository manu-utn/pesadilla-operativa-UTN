#ifndef UTILS_XLOG_H_
#define UTILS_XLOG_H_

#define COLOR_RESET "\x1b[0m"
#define COLOR_ERROR "\x1b[31m"       // ROJO
#define COLOR_PAQUETE "\x1b[32m"     // VERDE
#define COLOR_TAREA "\x1b[33m"       // AMARILLO
#define COLOR_CONEXION "\x1b[34m"    // AZUL
#define COLOR_RECURSOS "\x1b[35m"    // MAGENTA
#define COLOR_SERIALIZADO "\x1b[36m" // CYAN
#define COLOR_INFO "\x1b[37m"        // BLANCO

#define xlog(color, texto, args...) log_info(logger, color texto COLOR_RESET, ##args);
#endif
