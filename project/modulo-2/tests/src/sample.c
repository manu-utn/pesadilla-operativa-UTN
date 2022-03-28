#include "sample.h"
// usamos las funciones del modulo que contiene a los tests
#include "../../include/sample.h"

#include <cspecs/cspec.h>
#include <libshared.h>
#include <libstatic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

context(test_commons) {
  describe("Tipos primitivos"){
    it("true should be true"){should_bool(true) be equal to(true);
}
end

  it("Si el nombre es carlitos entonces se llama carlitos"){
  should_string("carlitos") be equal to("carlitos");
}
end

  it("Si el nombre es carlitos entonces NO se llama fede"){
  should_string("fede") be equal to("carlitos");
}
end

  it("Segun la static lib, cuando alguien grita dice aaa"){
  should_string(gritar()) be equal to("aaa");
}
end

  it("Segun este modulo, el numero loco es el 666"){
  should_int(numero_loco()) be equal to(666);
}
end
}
end
}
