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

void inicializar_archivo_swap(int pid, int tamanio, char* path) {
  char* filename = string_new();
  char* extension = ".swap";
  char* contenido = string_repeat('0', tamanio);
  size_t resultado;

  string_append(&filename, path);
  string_append(&filename, "/");
  string_append(&filename, string_itoa(pid));
  string_append(&filename, extension);

  FILE* fd = fopen(filename, "w+");

  if (fd == NULL) {
    log_error(logger, "Error al crear el archivo swap");
  }

  resultado = fwrite(contenido, sizeof(char*), tamanio, fd);

  if (resultado != tamanio) {
    log_error(logger, "No se ha inicializado correctamente el archivo.");
  } else {
    log_info(logger, "Archivo %s creado correctamente.", filename);
  }

  if (fclose(fd) != 0) {
    log_error(logger, "No se ha podido cerrar el fichero.\n");
  }
}

void eliminar_archivo_swap(int pid, int tamanio, char* path) {
  char* filename = string_new();
  char* extension = ".swap";

  string_append(&filename, path);
  string_append(&filename, "/");
  string_append(&filename, string_itoa(pid));
  string_append(&filename, extension);

  if (remove(filename) == 0) {
    log_info(logger, "El archivo %s fue eliminado satisfactoriamente\n", filename);
  } else {
    log_error(logger, "No se pudo eliminar el archivo\n");
  }
}