# - Este makefile actúa de template para el Makefile cada módulo

CONFIGS=message-colors.mk log.mk functions.mk

-include $(DIR_BASE)/$(DIR_PROJECT)/Makefile.cfg
-include $(foreach config, $(CONFIGS), $(addprefix $(DIR_CONFIGS), $(config)))

$(DIRS): ; $(MKDIR) $@

# TODO: la primera manera no considero que sea la mejor, la segunda con `undefine` no funciona como se espera
# - Le asignamos el valor en las configs modulos.cfg para desacoplar las static/shared libraries
DIR_SHARED_LIBS?=
DIR_STATIC_LIBS?=
#undefine DIR_STATIC_LIBS

####################################################################################################################
# rani you're the best one :)
# https://www.gnu.org/software/make/manual/html_node/Secondary-Expansion.html

# 1. Guardamos el nombre de las dependencias de las shared/static libraries
# - Agarra el nombre de la biblioteca Ej. `static-pepon` y convierte esta cadena `static-peon/bin/static-pepon.a`
#
# - Con (lib:%=%/bin/lib$(notdir $(lib)).ext) le agrega al final de cada ruta `/bin/lib/nombre.ext`
LIB_DEPS=$(foreach lib,$(DIR_SHARED_LIBS),$(lib:%=%/bin/lib$(notdir $(lib)).so)) \
	$(foreach lib,$(DIR_STATIC_LIBS),$(lib:%=%/bin/lib$(notdir $(lib)).a))

# 2. Cada dependencia (ej: ../static/bin/libstatic.a) se actualiza solo si se modifica algun fichero de su src/ o include/
# - Hara un nuevo build de cada shared/static library siempre que alguno de sus archivo fuente .c o archivos cabecera .h sean modificadas
# (compara el timestamp del binario .so o .a de sus codigos fuente .c y .h, si los binarios estan desactualizados
# entonces ejecuta la orden/recipe para el build)
#
# - El target `LID_DEPS` contiene la ruta de los binarios (.so y .a) lo reutiliza de la macro
#
# - La dependencia $$(shell find ...) reemplaza de la ruta que contiene el target el `bin/` por `/src`  usando la macro $@
# y los busca con `find` (comando de linux)
#
# - La orden/recipe `make -C ruta..` ejecuta el Makefile de la shared/static library
# - con `patsubt` remueve de la ruta el bin/
# - con $(dir $@) remueve el nombre del fichero de la ruta, solo queda una ruta
.SECONDEXPANSION:
$(LIB_DEPS): $$(shell find $$(patsubst %bin/,%src/,$$(dir $$@)) -iname "*.c") \
	$$(shell find $$(patsubst %bin/,%include/,$$(dir $$@)) -iname "*.h")
	-make --no-print-directory -C $(patsubst %bin/,%,$(dir $@))
####################################################################################################################

$(DIR_BIN)/$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)
	@echo $(LOG) "Enlazamos los objetos (\"$(notdir $^)\") para crear el ejecutable ($(notdir $@)) en $(dir $@)" $(MSG_OK)

# TODO: need refactor
# 1. Evitamos recursividad: chequeando si el binario a crear no es del modulo static, entonces solicitamos que se cree primero
# 2. Evitamos compilar varias veces la static lib: chequeando si ya se creo, entonces no pida crearla por cada .c que convierte en .o
# (evaluar porque en este segundo no anduvo la alternativa con wildcard)
$(OBJ): $(DIR_OBJ)/%.o: $(LIB_DEPS) $(DIR_SRC)/%.c $(DIR_DEP)/%.d | $(DIR_DEP)
ifeq (, $(shell which clang-format))
	$(error No tenes instalado el package clang-format ejecuta `make install` e intentalo de nuevo)
endif

#ifneq (static,$(notdir $(CURDIR)))
#	@if [ ! -f $(DIR_STATIC_LIB)/$(DIR_BIN)/lib_mylib.a  ]; \
#	then \
#		$(MAKE) --no-print-directory -C $(DIR_STATIC_LIB) compile_static; \
#	fi
#endif

	$(call apply_clang_format,$(DIR_SRC)/$*.c)
# redireccionamos el stderr al stdout con 2>&1 y el stodut a error.log con 1>>error.log (es importante el orden en que se hace)
	if $(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) -o $(DIR_OBJ)/$*.o -c $(DIR_SRC)/$*.c 1>>$(DIR_COMPILE_LOGS)/error.log 2>&1; \
	then \
		mv -f $(DIR_DEP)/$*.tmp.d $(DIR_DEP)/$*.d && touch $@;\
		echo $(LOG) "Compilamos el archivo fuente ($(notdir $<)) en objeto en $(dir $@)" $(MSG_OK);\
		echo $(LOG) "Se modificó el archivo ($?)" $(MSG_OK);\
	else \
		echo $(LOG) "Ocurrio un error al compilar $(OBJ)" $(MSG_ERROR);\
		sleep 2 && exit 1;\
	fi
	@echo ""
# --------------------------------------------------------------------

compile:$(DIRS) $(DIR_BIN)/$(BIN) ## Compilar módulo

test:
	@$(MAKE) --no-print-directory -C tests compile exec

clean-base: ## Remover ejecutables, objetos y dependencias
	@-$(RM) $(DIR_BIN)/*.out
	@-$(RM) $(DIR_BIN)/*.a
	@-$(RM) $(DIR_OBJ)/*.o
	@-$(RM) $(DIR_LOGS)/*.txt
	@-$(RM) $(DIR_LOGS)/*.log
	@-$(RM) $(DIR_DEP)/*.d
	@-$(RM) $(DIR_DEP)/*.tmp.d

	@echo "Removiendo ejecutable, objetos y dependencias desde $(CURRENT_MODULE_NAME)" $(MSG_OK)
ifneq ($(CURRENT_MODULE_NAME),tests)
	@$(MAKE) --no-print-directory -C tests clean
endif

e exec: ## Ejecutar binario
	@-$(CURRENT_PATH)/$(DIR_BIN)/$(BIN)

memcheck: ## Ejecutar Memcheck con la aplicación dentro del contenedor Docker
	@-valgrind $(MEMCHECK_FLAGS) --log-file="logs/memcheck.log" $(DIR_BIN)/$(BIN)
ifneq (, $(shell which valgreen))
	@-valgreen $(DIR_BIN)/$(BIN)
endif

debug:
	@-$(DEBUGGER) $(CURRENT_PATH)/$(DIR_BIN)/$(BIN)

$(DEP):
-include $(wildcard $(DEP))

w watch: ## Observar cambios en /src /include y compilar automáticamente
	$(info Observando cambios en la aplicación...)
	@while true; do $(MAKE) -q --no-print-directory || $(MAKE) --no-print-directory; sleep 1; done

h help: ## Muestra menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

# necesario para evitar los warnings de GNU Make por override de targets
# de makefiles que incluyen a este makefile
# los warnings hubieran sido
# - warning: overriding recipe for target nombre,
# - warning: ignoring old recipe for target nombre
%: %-base
	@true

# necesario para evitar
# el warning hubiera sido "make: *** No rule to make target 'nombre'.  Stop."
%:
	@true

.PHONY: e exec clean-base compile h help memcheck tests
