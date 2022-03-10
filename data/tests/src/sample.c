#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cspecs/cspec.h>
#include "sample.h"

context(test_commons) {
  describe("Tipos primitivos"){
    it("true should be true"){
      should_bool(true) be equal to(true);
    } end

        it("Si el nombre es carlitos entonces se llama carlitos"){
      should_string("carlitos") be equal to("carlitos");
    } end

    it("Si el nombre es carlitos entonces NO se llama fede") {
      should_string("fede") be equal to("carlitos");
    } end

} end

    describe("Commons"){
    it("El nombre de pikachu es pikachu"){
        Pokemon *poke = crear_pokemon("pikachu", 10, 10);
        should_string(poke->nombre) be equal to("pikachu");
        free_pokemon(poke);
    } end

  } end
}

static Pokemon *crear_pokemon(char *nombre, int fuerza, int velocidad) {
  Pokemon *pokemon = malloc(sizeof(Pokemon));
  pokemon->nombre = strdup(nombre);
  pokemon->fuerza = fuerza;
  pokemon->velocidad = velocidad;

  return pokemon;
}

static void free_pokemon(Pokemon *pokemon) {
  free(pokemon->nombre);
  free(pokemon);
}
