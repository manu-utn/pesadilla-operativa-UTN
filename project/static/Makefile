# TODO: fundamental respetar el orden de los includes
-include modulo.cfg
-include ../modulos.cfg
#-include ../Makefile

# opciones del comando `ar`
#
# r: agrega/actualiza ficheros en el archivo .a (si alguno no existe, lanzará un error)
# c: crea la biblioteca pero tiene que existir el .a
# s: agrega/actualiza un índice del archivo que usará el compilador

compile_static: $(DIR_STATIC_BIN)/$(STATIC_BIN)

$(DIR_STATIC_BIN)/$(STATIC_BIN): $(DIR_OBJ)/$(STATIC_LIB).o
	@echo "Creando biblioteca estática para compartir código entre módulos..." $(MSG_OK)
	@ar $(AR_OPTIONS) $@ $^

.PHONY: compile_static
