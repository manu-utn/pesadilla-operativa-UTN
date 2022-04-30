#ifndef UTILS_XLOG_H_
#define UTILS_XLOG_H_

#define COLOR_RESET "\x1b[0m"
#define COLOR_VERDE "\x1b[32m"
#define COLOR_ROJO "\x1b[31m"
#define COLOR_AMARILLO "\x1b[33m"
#define COLOR_AZUL "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_BLANCO "\x1b[37m"

#define xlog(color, texto, args...) log_info(logger, color texto COLOR_RESET, ##args);
#endif
