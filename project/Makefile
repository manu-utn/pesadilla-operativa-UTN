# - Este makefile actúa de template para el Makefile cada módulo

-include modulos.cfg
-include ../../.config/message-colors.mk

$(DIRS): ; $(MKDIR) $@

# --------------------------------------------------------------------

$(DIR_BIN)/$(BIN): $(OBJ)
	@$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)
	@echo "Enlazamos los objetos (\"$(notdir $^)\") para crear el ejecutable ($(notdir $@)) en $(dir $@)" $(MSG_OK)

# TODO: need refactor
# 1. Evitamos recursividad: chequeando si el binario a crear no es del modulo static, entonces solicitamos que se cree primero
# 2. Evitamos compilar varias veces la static lib: chequeando si ya se creo, entonces no pida crearla por cada .c que convierte en .o
# (evaluar porque en este segundo no anduvo la alternativa con wildcard)
$(OBJ): $(DIR_OBJ)/%.o: $(DIR_SRC)/%.c $(DIR_DEP)/%.d | $(DIR_DEP)
ifneq (, $(shell which clang-format))
	@clang-format $(DIR_SRC)/$*.c --style=file -i
	@echo "Formateando archivo $(DIR_SRC)/$*.c con clang-format.." $(MSG_OK)
endif
ifneq ($(notdir $(CURDIR)),static)
#ifeq ("$(wildcard $(DIR_STATIC_LIB)/$(DIR_BIN)/lib_mylib.a)","")
	@@[ -f $(DIR_STATIC_LIB)/$(DIR_BIN)/lib_mylib.a  ] || $(MAKE) --no-print-directory -C $(DIR_STATIC_LIB) compile_static
#endif
endif
	@$(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) -c $(DIR_SRC)/$*.c -o $(DIR_OBJ)/$*.o
	@mv -f $(DIR_DEP)/$*.tmp.d $(DIR_DEP)/$*.d && touch $@ # se ejecuta si no hubo error de compilación

	@echo "Compilamos el archivo fuente ($(notdir $<)) en objeto en $(dir $@)" $(MSG_OK)
	@echo "Se modificó el archivo ($?)" $(MSG_OK)
	@echo ""

# --------------------------------------------------------------------

compile:$(DIRS) $(DIR_BIN)/$(BIN) ## Compilar módulo

test:
	@$(MAKE) --no-print-directory -C tests compile exec

clean-base: ## Remover ejecutables, objetos y dependencias
	@-$(RM) $(DIR_BIN)/*.out
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
