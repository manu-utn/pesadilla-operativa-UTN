#include "libstatic.h"
#include "memoria.h"

int crear_punto_de_montaje(char* path) {
  int e;
  struct stat info;

  log_info(logger, "Preparando punto de montaje.");
  e = stat(path, &info);

  if (e == 0) {
    if (info.st_mode & S_IFREG) {
      log_error(logger, "El punto de montaje es un archivo.");
      return 0;
    }
    if (info.st_mode & S_IFDIR)
      log_info(logger, "Punto de montaje encontrado.");
  } else {
    if (errno == ENOENT) {
      log_warning(logger, "El punto de montaje no existe. Se creará el directorio.");
      e = mkdir(path, ACCESSPERMS | S_IRWXU);
      if (e != 0) {
        log_error(logger, "Se produjo un error al crear el directorio. [%d - %s]", errno, strerror(errno));
        return 0;
      } else
        log_info(logger, "El directorio se creó satisfactoriamente.");
    } else {
      log_error(logger, "Se produjo un error accediendo al punto de montaje. [%d - %s]", errno, strerror(errno));
      return 0;
    }
  }
  return 1;
}

char* get_filepath(char* file, char* path, int pid) {
  char* extension = ".swap";

  string_append(&file, path);
  string_append(&file, "/");
  string_append(&file, string_itoa(pid));
  string_append(&file, extension);

  return file;
}

void inicializar_archivo_swap(uint32_t pid, uint32_t tamanio) {
  char* filename = string_new();
  uint32_t contenido = 0000;
  uint32_t tamanio_archivo = tamanio / sizeof(uint32_t);
  char* path = obtener_path_archivos_swap();
  int desplazamiento = 0;

  filename = get_filepath(filename, path, pid);

  FILE* fd = fopen(filename, "wb");

  if (fd == NULL) {
    log_error(logger, "Error al crear el archivo swap");
  }

  for (int i = 0; i < tamanio_archivo; i++) {
    fseek(fd, desplazamiento, SEEK_SET);
    fwrite(&contenido, sizeof(uint32_t), 1, fd);
    desplazamiento = desplazamiento + sizeof(uint32_t);
  }

  // resultado = fwrite(&contenido, sizeof(uint32_t), tamanio_archivo, fd);

  if (desplazamiento != tamanio) {
    log_error(logger, "No se ha inicializado correctamente el archivo.");
  } else {
    log_info(logger, "Archivo %s creado correctamente.", filename);
  }

  if (fclose(fd) != 0) {
    log_error(logger, "No se ha podido cerrar el fichero.\n");
  }
}

void eliminar_archivo_swap(uint32_t pid) {
  char* filename = string_new();
  char* path = obtener_path_archivos_swap();

  filename = get_filepath(filename, path, pid);

  if (remove(filename) == 0) {
    log_info(logger, "El archivo %s fue eliminado satisfactoriamente\n", filename);
  } else {
    log_error(logger, "No se pudo eliminar el archivo\n");
  }
}

void escribir_archivo_swap(char* filepath, uint32_t* datos, int num_pagina) {
  int retardo = obtener_retardo_swap();
  xlog(COLOR_INFO, "Retardo de escribir archivo swap en milisegundos: %d", retardo);
  usleep(retardo * 1000);

  FILE* fd = fopen(filepath, "rb+");
  int tamanio_pagina = obtener_tamanio_pagina_por_config();

  int desplazamiento = num_pagina * tamanio_pagina;
  int longitud_datos = tamanio_pagina / sizeof(uint32_t);

  xlog(COLOR_INFO, "Numero de pagina %d y tamanio de pagina %d", num_pagina, tamanio_pagina);

  xlog(COLOR_INFO, "Se desplazo %d en el archivo %s", desplazamiento, filepath);

  for (int i = 0; i < longitud_datos; i++) {
    fseek(fd, desplazamiento, SEEK_SET);
    uint32_t dato = datos[i];
    fwrite(&dato, sizeof(uint32_t), 1, fd);
    desplazamiento = desplazamiento + sizeof(uint32_t);
  }

  // fseek(fd, desplazamiento, SEEK_SET);

  // // log_info(logger, "Se quieren escribir %d bytes", sizeof(uint32_t) * tamanioPagina);

  // fwrite(datos, sizeof(datos), 1, fd);

  fclose(fd);
}

uint32_t* leer_archivo_swap(char* filepath, int num_pagina) {
  int retardo = obtener_retardo_swap();
  xlog(COLOR_INFO, "Retardo de leer archivo swap en milisegundos: %d", retardo);
  usleep(retardo * 1000);

  FILE* fd = fopen(filepath, "rb+");
  int tamanio_pagina = obtener_tamanio_pagina_por_config();
  int tamanio_datos = tamanio_pagina / sizeof(uint32_t);

  uint32_t datos[tamanio_datos];

  int desplazamiento = num_pagina * tamanio_pagina;
  xlog(COLOR_INFO, "Numero de pagina %d y tamanio de pagina %d", num_pagina, tamanio_pagina);

  xlog(COLOR_INFO, "Se desplazo %d en el archivo %s", desplazamiento, filepath);

  for (int i = 0; i < tamanio_datos; i++) {
    fseek(fd, desplazamiento, SEEK_SET);
    fread(&datos[i], sizeof(uint32_t), 1, fd);
    desplazamiento = desplazamiento + sizeof(uint32_t);
  }

  // fseek(fd, desplazamiento, SEEK_SET);
  // fread(&datos, sizeof(uint32_t), tamanio_datos, fd);

  xlog(COLOR_TAREA, "Leimos de Swap %d", datos[0]);

  fclose(fd);

  uint32_t* datos_leidos = (uint32_t*)datos;

  return datos_leidos;
}

void liberar_estructuras_en_swap(int pid) {
  xlog(COLOR_CONEXION, "SWAP recibió solicitud de Kernel para liberar recursos de un proceso");
  eliminar_archivo_swap(pid);
}

void escribir_datos_de_swap_en_marco(t_marco* marco) {
  int tamanio_pagina = obtener_tamanio_pagina_por_config();
  uint32_t* datos;
  char* filename = string_new();
  char* path = obtener_path_archivos_swap();

  filename = get_filepath(filename, path, marco->pid);

  int numero_entrada_primer_nivel = marco->entrada_segundo_nivel->numero_entrada_primer_nivel;
  int numero_entrada_segundo_nivel = marco->entrada_segundo_nivel->entrada_segundo_nivel;
  int cantidad_entradas_segundo_nivel = obtener_cantidad_entradas_por_tabla_por_config();

  int num_pagina = numero_entrada_primer_nivel * cantidad_entradas_segundo_nivel + numero_entrada_segundo_nivel;

  datos = leer_archivo_swap(filename, num_pagina);
  // TODO: Revisar a partir de aca

  int limite = tamanio_pagina / 4;
  int i = 0;
  uint32_t direccion = marco->num_marco * tamanio_pagina;

  // uint32_t* datos_leidos = (uint32_t*)datos;
  int j = 0;
  while (i < limite) {
    uint32_t dato = datos[j];
    escribir_dato(direccion + i, dato);
    j++;
    i += 4;
  }
}

// Funciones que se usan para la suspension

void escribir_datos_de_marcos_en_swap(t_list* marcos) {
  list_iterate(marcos, (void*)escribir_marco_en_swap);
}

// TODO: REVISAR QUE ESTA MAL PENSADA CONCEPTUALMENTE LA FUNCION (VA A BUSCAR DE BYTE Y RECIBE 4 B)
void escribir_marco_en_swap(t_marco* marco) {
  int tamanio_pagina = obtener_tamanio_pagina_por_config();
  int limite = tamanio_pagina / 4;
  int i = 0;
  uint32_t direccion = marco->num_marco * tamanio_pagina;

  uint32_t datos[limite];

  int j = 0;

  while (i < limite) {
    uint32_t dato = buscar_dato_en_memoria(direccion + i);
    datos[j] = dato;
    j++;
    i += 4;
  }

  xlog(COLOR_TAREA, "Leimos del Marco %s", (char*)datos);

  char* filename = string_new();
  char* path = obtener_path_archivos_swap();

  filename = get_filepath(filename, path, marco->pid);

  int numero_entrada_primer_nivel = marco->entrada_segundo_nivel->numero_entrada_primer_nivel;
  int numero_entrada_segundo_nivel = marco->entrada_segundo_nivel->entrada_segundo_nivel;
  int cantidad_entradas_segundo_nivel = obtener_cantidad_entradas_por_tabla_por_config();

  int num_pagina = numero_entrada_primer_nivel * cantidad_entradas_segundo_nivel + numero_entrada_segundo_nivel;

  escribir_archivo_swap(filename, (uint32_t*)datos, num_pagina);
}