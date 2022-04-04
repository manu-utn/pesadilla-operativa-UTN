#include "libstatic.h"
#include "utils-servidor.h"
#include <stdio.h>

t_config* iniciar_config(char* config) {
  return config_create(config);
}
