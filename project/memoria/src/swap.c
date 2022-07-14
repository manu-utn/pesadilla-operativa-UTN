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
  FILE* fd = fopen(filepath, "rb+");
  int tamanioPagina = obtener_tamanio_pagina_por_config();

  int desplazamiento = numPagina * tamanioPagina;
  int longitudDatos = string_length((char*)datos);

  log_info(logger, "Se desplazo %d en el archivo %s", desplazamiento, filepath);

  fseek(fd, desplazamiento, SEEK_SET);

  // log_info(logger, "Se quieren escribir %d bytes", sizeof(uint32_t) * tamanioPagina);

  fwrite(datos, sizeof(char), longitudDatos, fd);

  fclose(fd);
}

void leer_archivo_swap(char* filepath, int numPagina) {
  FILE* fd = fopen(filepath, "rb+");
  int tamanioPagina = obtener_tamanio_pagina_por_config();
  void* datos[tamanioPagina];

  int desplazamiento = numPagina * tamanioPagina;

  log_info(logger, "Se desplazo %d en el archivo %s", desplazamiento, filepath);

  fseek(fd, desplazamiento, SEEK_SET);

  fread(datos, sizeof(char), tamanioPagina, fd);

  log_info(logger, "Leimos %s", (char*)datos);

  fclose(fd);
}

void liberar_estructuras_en_swap(int pid) {
  xlog(COLOR_CONEXION, "SWAP recibió solicitud de Kernel para liberar recursos de un proceso");
  eliminar_archivo_swap(pid);
}


void escribir_datos_de_marcos_en_swap(t_list* marcos) {
  list_iterate(marcos, (void*)escribir_marco_en_swap);
}

// TODO: REVISAR QUE ESTA MAL PENSADA CONCEPTUALMENTE LA FUNCION (VA A BUSCAR DE BYTE Y RECIBE 4 B)
void escribir_marco_en_swap(t_marco* marco) {
  int tamanioPagina = obtener_tamanio_pagina_por_config();
  uint32_t datos[tamanioPagina];

  for (int i = 0; i < tamanioPagina; i++) {
    uint32_t dato = buscar_dato_en_memoria((marco->num_marco * tamanioPagina) + i);
    datos[i] = dato;
  }

  char* filename = string_new();
  char* path = obtener_path_archivos_swap();

  filename = get_filepath(filename, path, marco->pid);

  int num_pagina = marco->entrada_segundo_nivel->entrada_segundo_nivel;

  escribir_archivo_swap(filename, (void*)datos, num_pagina);

  marco->entrada_segundo_nivel->bit_modif = 0;
  // TODO Evaluar si deberia cambiar el bit de uso y el bit de presencia
  marco->pid = 0;
  marco->ocupado = 0;
}