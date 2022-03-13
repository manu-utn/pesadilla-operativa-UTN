# - Este makefile actúa de template para el Makefile cada módulo
-include modulos.cfg

$(DIRS): ; $(MKDIR) $@

# --------------------------------------------------------------------

$(DIR_BIN)/$(BIN): $(OBJ)
	$(info Enlazamos los objetos ("$(notdir $^)") para crear el ejecutable ($(notdir $@)) en $(dir $@))
	@$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ): $(DIR_OBJ)/%.o: $(DIR_SRC)/%.c $(DIR_DEP)/%.d | $(DIR_DEP)
	$(info Compilamos el archivo fuente ($(notdir $<)) en objeto en $(dir $@))
	$(info Se modificó el archivo ($?))
	@$(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) -c $(DIR_SRC)/$*.c -o $(DIR_OBJ)/$*.o
	@mv -f $(DIR_DEP)/$*.tmp.d $(DIR_DEP)/$*.d && touch $@ # se ejecuta si no hubo error de compilación


# --------------------------------------------------------------------

static_lib:$(DIR_BIN)/$(BIN)
	@$(MAKE) --no-print-directory -C $(DIR_STATIC_LIB)

# TODO: Posible refactor (?)
ifeq ($(BIN_FILE_EXTENSION),.a)
compile: $(DIRS) $(DIR_BIN)/$(BIN) ## Compilar módulo
else
compile: $(DIRS) static_lib $(DIR_BIN)/$(BIN)
endif

tests:
	@$(MAKE) --no-print-directory -C tests compile exec

c clean: ## Remover ejecutables, objetos y dependencias
	$(info Removiendo ejecutable, objetos y dependencias desde $(CURRENT_MODULE_NAME))
	@-$(RM) $(DIR_BIN)/*.out
	@-$(RM) $(DIR_OBJ)/*.o
	@-$(RM) $(DIR_LOGS)/*.{txt,log}
	@-$(RM) $(DIR_DEP)/*.{d,tmp.d}

e exec: ## Ejecutar binario
	@-$(DIR_BIN)/$(BIN)

memcheck: ## Ejecutar Memcheck con la aplicación dentro del contenedor Docker
	@valgrind $(MEMCHECK_FLAGS) $(DIR_BIN)/$(BIN)

$(DEP):
-include $(wildcard $(DEP))

w watch: ## Observar cambios en /src /include y compilar automáticamente
	$(info Observando cambios en la aplicación...)
	@while true; do $(MAKE) -q --no-print-directory || $(MAKE) --no-print-directory; sleep 1; done

h help: ## Muestra menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

.PHONY: c clean e exec compile h help memcheck static_lib tests
