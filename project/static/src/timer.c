#include "timer.h"

void *timer_contar() {
  // en milisegundos.. 1000 milisegundos = 1 segundo
  xlog(COLOR_INFO, "[TIMER]: Contando..");

  TIMER.timer_inicio = clock();
  while (TIMER.timer_estado)
    ;
  TIMER.timer_fin = clock();

  TIMER.tiempo_total = TIMER.timer_fin - TIMER.timer_inicio;

  pthread_exit(NULL);
}

void timer_iniciar() {
  TIMER.timer_estado = TIMER_ACTIVADO;

  pthread_t th;
  pthread_create(&th, NULL, timer_contar, NULL), pthread_detach(th);
}

void timer_detener() {
  TIMER.timer_estado = TIMER_DESACTIVADO;
}

void timer_imprimir() {
  xlog(COLOR_INFO, "[TIMER]: timer_inicio=%0.f ", (float)TIMER.timer_inicio);
  xlog(COLOR_INFO, "[TIMER]: timer_fin=%0.f ", (float)TIMER.timer_fin);
  xlog(COLOR_INFO, "[TIMER]: tiempo_total=%d", TIMER.tiempo_total);
}

int microsegundos_humanizar(int microsegundos) {
  // usleep: usa microsegundos (1.000.000 microsegundos = 1 segundo)
  // clock: usa milisegundos (1.000 milisegundos = 1 segundo)
  // emulamos 3.000 microsegundos a 3 segundos que representan 3.000.000 microsegundos
  return microsegundos * 1000;
}

void simular_bloqueo_en_segundos(int tiempo_microsegundos) {
  usleep(microsegundos_humanizar(tiempo_microsegundos));
}
