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
  void* contenido = string_repeat('0', tamanio);
  char* path = obtener_path_archivos_swap();

  size_t resultado;

  filename = get_filepath(filename, path, pid);

  FILE* fd = fopen(filename, "wb");

  if (fd == NULL) {
    log_error(logger, "Error al crear el archivo swap");
  }

  resultado = fwrite(contenido, sizeof(char), tamanio, fd);

  if (resultado != tamanio) {
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

void escribir_archivo_swap(char* filepath, void* datos, int numPagina) {
  int retardo = obtener_retardo_swap();
  xlog(COLOR_INFO, "Retardo de escribir archivo swap en milisegundos: %d", retardo);
  usleep(retardo * 1000);

  FILE* fd = fopen(filepath, "rb+");
  int tamanio_pagina = obtener_tamanio_pagina_por_config();

  int desplazamiento = numPagina * tamanio_pagina;
  int longitudDatos = string_length((char*)datos);

  log_info(logger, "Se desplazo %d en el archivo %s", desplazamiento, filepath);

  fseek(fd, desplazamiento, SEEK_SET);

  // log_info(logger, "Se quieren escribir %d bytes", sizeof(uint32_t) * tamanioPagina);

  fwrite(datos, sizeof(char), longitudDatos, fd);

  fclose(fd);
}

char* leer_archivo_swap(char* filepath, int numPagina) {
  int retardo = obtener_retardo_swap();
  xlog(COLOR_INFO, "Retardo de leer archivo swap en milisegundos: %d", retardo);
  usleep(retardo * 1000);

  FILE* fd = fopen(filepath, "rb+");
  int tamanio_pagina = obtener_tamanio_pagina_por_config();
  void* datos[tamanio_pagina];

  int desplazamiento = numPagina * tamanio_pagina;

  log_info(logger, "Se desplazo %d en el archivo %s", desplazamiento, filepath);

  fseek(fd, desplazamiento, SEEK_SET);

  fread(datos, sizeof(char), tamanio_pagina, fd);

  log_info(logger, "Leimos %s", (char*)datos);

  fclose(fd);

  char* datosLeidos = (char*)datos;

  return datosLeidos;
}

void liberar_estructuras_en_swap(int pid) {
  xlog(COLOR_CONEXION, "SWAP recibió solicitud de Kernel para liberar recursos de un proceso");
  eliminar_archivo_swap(pid);
}

void escribir_datos_de_swap_en_marco(t_marco* marco) {
  int tamanio_pagina = obtener_tamanio_pagina_por_config();
  char* datos;
  char* filename = string_new();
  char* path = obtener_path_archivos_swap();

  filename = get_filepath(filename, path, marco->pid);

  int numero_entrada_primer_nivel = marco->entrada_segundo_nivel->numero_tabla_segundo_nivel;
  int numero_entrada_segundo_nivel = marco->entrada_segundo_nivel->entrada_segundo_nivel;
  int cantidad_entradas_segundo_nivel = obtener_cantidad_entradas_por_tabla_por_config();

  int num_pagina = numero_entrada_primer_nivel * cantidad_entradas_segundo_nivel + numero_entrada_segundo_nivel;

  datos = leer_archivo_swap(filename, num_pagina);
  // char* datosTexto = (char*)datos;

  int limite = tamanio_pagina / 4;
  int i = 0;
  uint32_t direccion = marco->num_marco * tamanio_pagina;

  while (i < limite) {
    uint32_t dato = datos[i];
    escribir_dato(direccion + i, dato);
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

  uint32_t datos[tamanio_pagina];

  while (i < limite) {
    uint32_t dato = buscar_dato_en_memoria(direccion + i);
    datos[i] = dato;
    i += 4;
  }

  char* filename = string_new();
  char* path = obtener_path_archivos_swap();

  filename = get_filepath(filename, path, marco->pid);

  int numero_entrada_primer_nivel = marco->entrada_segundo_nivel->numero_tabla_segundo_nivel;
  int numero_entrada_segundo_nivel = marco->entrada_segundo_nivel->entrada_segundo_nivel;
  int cantidad_entradas_segundo_nivel = obtener_cantidad_entradas_por_tabla_por_config();

  int num_pagina = numero_entrada_primer_nivel * cantidad_entradas_segundo_nivel + numero_entrada_segundo_nivel;

  escribir_archivo_swap(filename, (void*)datos, num_pagina);

  marco->entrada_segundo_nivel->bit_presencia = 0;
  marco->entrada_segundo_nivel->bit_modif = 0;
  marco->pid = 0;
  marco->ocupado = MARCO_LIBRE;
}