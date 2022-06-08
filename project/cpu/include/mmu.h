#ifndef MMU_H_
#define MMU_H_

#include <time.h>
#include "cpu.h"

typedef struct{
    uint32_t    pagina;
    uint32_t    marco;
    uint64_t    time;
}t_entrada_tlb;


t_list      *   tlb;
uint32_t        cantidad_entradas_tlb;
char        *   reemplazo_tlb;


void                iniciar_tlb                 (void);
void                limpiar_tlb                 (void);
uint32_t            obtener_numero_pagina       (uint32_t           direccion_logica);
uint32_t            obtener_entrada_1er_nivel   (uint32_t           numero_pagina,      uint32_t    entradas_por_tabla);
uint32_t            obtener_entrada_2do_nivel   (uint32_t           numero_pagina,      uint32_t    entradas_por_tabla);
uint32_t            obtener_desplazamiento      (uint32_t           direccion_logica,   uint32_t    numero_pagina);
t_entrada_tlb   *   obtener_entrada_tlb         (uint32_t           pagina,             uint32_t    marco);
void                agregar_entrada_tlb         (t_entrada_tlb  *   entrada_tlb);
void                realizar_reemplazo_fifo     (t_entrada_tlb  *   entrada_tlb);
void                realizar_reemplazo_lru      (t_entrada_tlb  *   entrada_tlb);
int                 existe_pagina_en_tlb        (uint32_t           pagina);
int                 busco_index_oldest          (void);


//PARA PRUEBAS -> despues borrar
void                prueba_datos_tlb            (void);

#endif /* MMU_H_ */