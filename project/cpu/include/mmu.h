#ifndef MMU_H_
#define MMU_H_

#include <time.h>
#include "cpu.h"

typedef struct{
    uint32_t    pid;
    uint32_t    pagina;
    uint32_t    marco;
    uint32_t    time_sec;
    uint64_t    time_nanosec;
}t_entrada_tlb;


t_list      *   tlb;
uint32_t        cantidad_entradas_tlb;
char        *   reemplazo_tlb;


void                iniciar_tlb                 (void);
void                limpiar_tlb                 (uint32_t           pid);
uint32_t            obtener_numero_pagina       (uint32_t           direccion_logica);
uint32_t            obtener_entrada_1er_nivel   (uint32_t           numero_pagina,      uint32_t    entradas_por_tabla);
uint32_t            obtener_entrada_2do_nivel   (uint32_t           numero_pagina,      uint32_t    entradas_por_tabla);
uint32_t            obtener_desplazamiento      (uint32_t           direccion_logica,   uint32_t    numero_pagina);
void                agregar_pagina_marco_tlb    (uint32_t           pagina,             uint32_t    marco,                  uint32_t pid);
t_entrada_tlb   *   obtener_entrada_tlb         (uint32_t           pagina,             uint32_t    marco,                  uint32_t pid);
void                agregar_entrada_tlb         (t_entrada_tlb  *   entrada_tlb);
void                realizar_reemplazo_fifo     (t_entrada_tlb  *   entrada_tlb);
void                realizar_reemplazo_lru      (t_entrada_tlb  *   entrada_tlb);
int                 existe_pagina_en_tlb        (uint32_t           pagina);
int                 busco_index_oldest          (void);
uint32_t            obtener_marco_tlb           (int                indice);
uint32_t            obtener_direccion_fisica    (uint32_t           desplazamiento,     uint32_t marco);

//PARA PRUEBAS -> despues borrar
t_list          *   prueba_crear_datos_tlb      (void);
void                realizar_pruebas_tlb        (t_list         *   tlb_prueba);
#endif /* MMU_H_ */