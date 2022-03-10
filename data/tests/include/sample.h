typedef struct{
  char* nombre;
  int fuerza;
  int velocidad;
} Pokemon;

static Pokemon* crear_pokemon(char* nombre, int fuerza, int velocidad);
static void free_pokemon(Pokemon* pokemon);
