-include Makefile.cfg
-include ../Makefile

# opciones del comando `ar`
#
# r: agrega/actualiza ficheros en el archivo .a (si alguno no existe, lanzará un error)
# c: crea la biblioteca pero tiene que existir el .a
# s: agrega/actualiza un índice del archivo que usará el compilador


b build: $(DIRS) $(DIR_BIN)/$(BIN)
#	@echo $(suffix $(BIN))
#	@echo "estoy en static"

$(DIR_BIN)/$(BIN): $(DIR_OBJ)/$(STATIC_LIB).o
	$(info Creando biblioteca estática para compartir código entre módulos...)
	@ar $(AR_OPTIONS) $@ $^

c clean:
	@-$(RM) $(DIR_BIN)/$(BIN)
