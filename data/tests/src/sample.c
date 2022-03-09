#include <stdbool.h>
#include <stdio.h>
#include <cspecs/cspec.h>

context(example) {
  describe("Hello world"){
    it("true should be true"){should_bool(true) be equal to(true);
    } end

} end
}
