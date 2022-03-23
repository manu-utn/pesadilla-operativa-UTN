# - Este makefile actúa de template para el Makefile cada módulo

-include modulos.cfg

$(DIRS): ; $(MKDIR) $@

# --------------------------------------------------------------------

$(DIR_BIN)/$(BIN): $(OBJ)
	$(info > Enlazamos los objetos ("$(notdir $^)") para crear el ejecutable ($(notdir $@)) en $(dir $@))
	@$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ): $(DIR_OBJ)/%.o: $(DIR_SRC)/%.c $(DIR_DEP)/%.d | $(DIR_DEP)
# TODO: need refactor
ifneq (, $(shell which clang-format))
	$(info > Formateando archivo $(DIR_SRC)/$*.c con clang-format..)
	@clang-format $(DIR_SRC)/$*.c --style=file -i
endif
# TODO: need refactor
# Ex. [ -f path/file ] && file_exist || file_not_exist
# Ex. [ -f path/file ] || file_not_exist
ifneq ($(notdir $(CURDIR)),static)
	@[ -f $(DIR_STATIC_LIB)/$(DIR_BIN)/lib_mylib.a  ] || \
		$(MAKE) --no-print-directory -C $(DIR_STATIC_LIB)
endif
	$(info > Compilamos el archivo fuente ($(notdir $<)) en objeto en $(dir $@))
	$(info > Se modificó el archivo ($?))
	@$(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) -c $(DIR_SRC)/$*.c -o $(DIR_OBJ)/$*.o
	@mv -f $(DIR_DEP)/$*.tmp.d $(DIR_DEP)/$*.d && touch $@ # se ejecuta si no hubo error de compilación
	@echo ""

# --------------------------------------------------------------------

compile:$(DIRS) $(DIR_BIN)/$(BIN) ## Compilar módulo

test:
	@$(MAKE) --no-print-directory -C tests compile exec

c clean: ## Remover ejecutables, objetos y dependencias
	$(info > Removiendo ejecutable, objetos y dependencias desde $(CURRENT_MODULE_NAME))
	@-$(RM) $(DIR_BIN)/*.out
	@-$(RM) $(DIR_OBJ)/*.o
	@-$(RM) $(DIR_LOGS)/*.txt
	@-$(RM) $(DIR_LOGS)/*.log
	@-$(RM) $(DIR_DEP)/*.d
	@-$(RM) $(DIR_DEP)/*.tmp.d
ifneq ($(CURRENT_MODULE_NAME),tests)
	@$(MAKE) --no-print-directory -C tests clean
endif

e exec: ## Ejecutar binario
	@-$(CURRENT_PATH)/$(DIR_BIN)/$(BIN)

memcheck: ## Ejecutar Memcheck con la aplicación dentro del contenedor Docker
	@valgrind $(MEMCHECK_FLAGS) $(DIR_BIN)/$(BIN)

debug:
	@$(DEBUGGER) $(CURRENT_PATH)/$(DIR_BIN)/$(BIN)

$(DEP):
-include $(wildcard $(DEP))

w watch: ## Observar cambios en /src /include y compilar automáticamente
	$(info Observando cambios en la aplicación...)
	@while true; do $(MAKE) -q --no-print-directory || $(MAKE) --no-print-directory; sleep 1; done

h help: ## Muestra menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

.PHONY: c clean e exec compile h help memcheck tests
