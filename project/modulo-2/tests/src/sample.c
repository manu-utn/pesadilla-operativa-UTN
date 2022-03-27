#include "sample.h"
#include <cspecs/cspec.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

context(test_commons) {
  describe("Tipos primitivos"){
    it("true should be true"){should_bool(true) be equal to(true);
}
end

  it("Si el nombre es carlitos entonces se llama carlitos")
{
  should_string("carlitos") be equal to("carlitos");
}
end

  it("Si el nombre es carlitos entonces NO se llama fede")
{
  should_string("fede") be equal to("carlitos");
}
end
}
end
}
