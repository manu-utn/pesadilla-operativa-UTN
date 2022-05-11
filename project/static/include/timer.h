#ifndef __MY_TIMER_H__
#define __MY_TIMER_H__

#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "xlog.h"

typedef enum { TIMER_ACTIVADO = 1, TIMER_DESACTIVADO = 0 } timer_estado;

typedef struct {
  clock_t timer_inicio, timer_fin;
  int tiempo_total;
  timer_estado timer_estado;
} pcb_timer_t;

pcb_timer_t TIMER;

void *timer_contar();
void timer_iniciar();
void timer_detener();
void timer_imprimir();
int microsegundos_humanizar(int microsegundos);
void simular_bloqueo_en_segundos(int tiempo_microsegundos);
#endif
