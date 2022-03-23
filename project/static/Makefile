-include modulo.cfg
-include ../Makefile

# opciones del comando `ar`
#
# r: agrega/actualiza ficheros en el archivo .a (si alguno no existe, lanzará un error)
# c: crea la biblioteca pero tiene que existir el .a
# s: agrega/actualiza un índice del archivo que usará el compilador

$(DIR_BIN)/$(BIN): $(DIR_OBJ)/$(STATIC_LIB).o
	$(info Creando biblioteca estática para compartir código entre módulos...)
	@ar $(AR_OPTIONS) $@ $^

c clean:
	$(info > Removiendo ejecutable, objetos y dependencias desde $(CURRENT_MODULE_NAME))
	@-$(RM) $(DIR_BIN)/*.a
	@-$(RM) $(DIR_OBJ)/*.o
	@-$(RM) $(DIR_LOGS)/*.txt
	@-$(RM) $(DIR_LOGS)/*.log
	@-$(RM) $(DIR_DEP)/*.d
	@-$(RM) $(DIR_DEP)/*.tmp.d
	@echo ""
ifneq ($(CURRENT_MODULE_NAME),tests)
	@$(MAKE) --no-print-directory -C tests clean
endif
