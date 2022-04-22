#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ./executable ./una/ruta/a/mi/archivo tamanio

char *read_file(char *filename);
char **split(char *string, char *seperators, int *count);

typedef struct {
  char *instruccion;
  uint32_t *args;
} t_instruccion;
// Seguramente sea modificado luego, pero al parecer solo se ingresan
// 2 enteros maximos

int main(int argc, char **argv) {
  // argc -> Cantidad de argumentos (se considera al ./nombre como el primer
  // argumento)
  // argv -> Array de string
  if (argc != 3) {
    printf("No pueden ingresarse mas ni menos de dos argumentos \n");
    return 1;
  }

  // argv[1] -> Path (ruta)
  // argv[2] -> Size (tamanio)
  char *path = malloc(sizeof(char) * strlen(argv[1]) + 1); // TODO: Need free
  // strcpy(path, argv[1]); -> Preferible usar memcpy
  memcpy(path, argv[1], strlen(argv[1]));

  int tamanio = atoi(argv[2]);
  // atoi se usa para convertir str a int

  printf("Path: %s\n", path);
  printf("Tamanio: %d\n\n", tamanio);

  // read the file contents of the file
  char *file_contents = read_file(path);
  // if there was an error reading the file, file_contents will be set to NULL,
  // handle the error gracefully with an error message and error return status
  if (file_contents == NULL) {
    printf("Error reading file.\n");
    return 1;
  }

  // output the file contents to verify they were read into the string correctly
  printf("File Contents:\n\n%s\n", file_contents);

  int count_instructions = 0;
  int count_sub_instructions = 0;
  char **split_instructions = split(file_contents, "\n", &count_instructions);
  char **split_sub_instructions;

  // print out the substrings, which should be each word of the sentence above
  for (int i = 0; i < count_instructions; i++) {
    printf("%s\n", split_instructions[i]);
    split_sub_instructions =
      split(split_instructions[i], " ", &count_sub_instructions);

    for (int it = 0; it < count_sub_instructions; it++) {
      printf("(%s)\n", split_sub_instructions[it]);
      // Aca deberia agregar 1 a 1 los argumentos a una t_instruccion
      // que luego sera agregrado a una lista de t_instruccion*
      if (it == 0) {
      }
    }
    for (int it = 0; it < count_sub_instructions; it++) {
      free(split_sub_instructions[it]);
    }
    free(split_sub_instructions);
  }

  // free the dynamically allocated space for each string
  for (int i = 0; i < count_instructions; i++) {
    free(split_instructions[i]);
  }

  // free the dynamically allocated space for the array of pointers to strings
  free(split_instructions);


  // free the dynamically allocated memory as a best practice to prevent a
  // memory leak from occurring
  free(file_contents);

  free(path);

  return 0;
}
// gcc *.c -g -Wall -o pruebaConsola


// Separates the string into substrings, splitting the string into substrings
// based on the separator characters (i.e separators).  The function returns an
// array of pointers to strings, dynamically allocated on the heap, and it
// effectively "returns" the number of these strings via pass-by-pointer using
// the parameter count.
char **split(char *string, char *seperators, int *count) {
  // get the length of the string
  int len = strlen(string);

  // use count to keep a count of the number of substrings
  *count = 0;

  // We make one pass of the string to first determine how many substrings
  // we'll need to create, so we can allocate space for a large enough array
  // of pointer to strings.  The variable i will keep track of our current
  // index in the string
  int i = 0;
  while (i < len) {
    // skip over the next group of separator characters
    while (i < len) {
      // keep incrementing i until the character at index i is NOT found in the
      // separators array, indicating we've reached the next substring to create
      if (strchr(seperators, string[i]) == NULL)
        break;
      i++;
    }

    // skip over the next group of substring (i.e. non-separator characters),
    // we'll use old_i to verify that we actually did detect non-separator
    // characters (perhaps we're at the end of the string)
    int old_i = i;
    while (i < len) {
      // increment i until the character at index i IS found in the separators
      // array, indicating we've reached the next group of separator
      // character(s)
      if (strchr(seperators, string[i]) != NULL)
        break;
      i++;
    }

    // if we did encounter non-seperator characters, increase the count of
    // substrings that will need to be created
    if (i > old_i)
      *count = *count + 1;
  }

  // allocate space for a dynamically allocated array of *count* number of
  // pointers to strings
  char **strings = malloc(sizeof(char *) * *count);

  // we'll make another pass of the string using more or less the same logic as
  // above, but this time we'll dynamically allocate space for each substring
  // and store the substring into this space
  i = 0;

  // buffer will temporarily store each substring, string_index will keep track
  // of the current index we are storing the next substring into using the
  // dynamically allocated array above
  char buffer[16384];
  int string_index = 0;
  while (i < len) {
    // skip through the next group of separators, exactly the same as above
    while (i < len) {
      if (strchr(seperators, string[i]) == NULL)
        break;
      i++;
    }

    // store the next substring into the buffer char array, use j to keep
    // track of the index in the buffer array to store the next char
    int j = 0;
    while (i < len) {
      if (strchr(seperators, string[i]) != NULL)
        break;

      buffer[j] = string[i];
      i++;
      j++;
    }

    // only copy the substring into the array of substrings if we actually
    // read in characters with the above loop... it's possible we won't if
    // the string ends with a group of separator characters!
    if (j > 0) {
      // add a null terminator on to the end of buffer to terminate the string
      buffer[j] = '\0';

      // calculate how much space to allocate... we need to be able to store
      // the length of buffer (including a null terminator) number of characters
      int to_allocate = sizeof(char) * (strlen(buffer) + 1);

      // allocate enough space using malloc, store the pointer into the strings
      // array of pointers at hte current string_index
      strings[string_index] = malloc(to_allocate);

      // copy the buffer into this dynamically allocated space
      strcpy(strings[string_index], buffer);

      // advance string_index so we store the next string at the next index in
      // the strings array
      string_index++;
    }
  }

  // return our array of strings
  return strings;
}

// Reads and stores the whole contents of the file with filename into a
// dynamically allocated char array on the heap, returns a pointer to this char
// array (or NULL if there was an error reading the file contents)/
char *read_file(char *filename) {
  // file pointer variable used to access the file
  FILE *file;

  // attempt to open the file in read mode
  file = fopen(filename, "r");

  // if the file fails to open, return NULL as an error return value
  if (file == NULL)
    return NULL;

  // move the file pointer to the end of the file
  fseek(file, 0, SEEK_END);

  // fseek(file) will return the current value of the position indicator,
  // which will give us the number of characters in the file
  int length = ftell(file);

  // move file pointer back to start of file so we can read each character
  fseek(file, 0, SEEK_SET);

  // dynamically allocate a char array to store the file contents, we add 1 to
  // length for the null terminator we will need to add to terminate the string
  char *string = malloc(sizeof(char) * (length + 1));

  // c will store each char we read from the string
  char c;

  // i will be an index into the char array string as we read each char
  int i = 0;

  // keep reading each char from the file until we reach the end of the file
  while ((c = fgetc(file)) != EOF) {
    // store char into the char array string
    string[i] = c;

    // increment i so we store the next char in the next index in the char array
    i++;
  }

  // put a null terminator as the final char in the char array to properly
  // terminate the string
  string[i] = '\0';

  // close the file as we are now done with it
  fclose(file);

  // return a pointer to the dynamically allocated string on the heap
  return string;
}
// ./pruebaConsola ~/Escritorio/Matias/so/pruebitas/pruebitas/pseudocodigo.txt 2
