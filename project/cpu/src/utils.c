#include "cpu.h"
#include "dir.h"
#include "sample.h"

void mock_datos_tlb() {
  t_entrada_tlb* entrada1 = malloc(sizeof(t_entrada_tlb));
  t_entrada_tlb* entrada2 = malloc(sizeof(t_entrada_tlb));
  t_entrada_tlb* entrada3 = malloc(sizeof(t_entrada_tlb));
  t_entrada_tlb* entrada4 = malloc(sizeof(t_entrada_tlb));

  entrada1->proceso = 1;
  entrada1->pagina = 1;
  entrada1->marco = 1;

  entrada2->proceso = 2;
  entrada2->pagina = 2;
  entrada2->marco = 2;

  entrada3->proceso = 3;
  entrada3->pagina = 3;
  entrada3->marco = 3;

  entrada3->proceso = 4;
  entrada3->pagina = 4;
  entrada3->marco = 4;

  list_add(tlb, entrada1);
  list_add(tlb, entrada2);
  list_add(tlb, entrada3);
  list_add(tlb, entrada4);
}