# 1er param: $1: recibe el nombre de una o varias reglas de un makefile
# 2do param: $2: recibe el nombre de un modulo del proyecto (es opcional)
define specific_module_cmd
	$(MAKE) --no-print-directory -C $(DIR_PROJECT)/$2 $1
endef

# 1er param: $1: recibe el nombre de una o varias reglas de un makefile
# ARGS: recibe multiples parametros por la terminal
define module_cmd
	$(MAKE) --no-print-directory -C $(DIR_PROJECT)/$(ARGS) $1
endef

# 1er param: $1 rutas de los archivos fuente .c
define create_ctags
	@-$(RM) $(PATH_CTAGS)
	ctags -o - --kinds-C=f -x \
	--_xformat="%{typeref} %{name}%{signature};" $1 | \
	tr ':' ' ' | sed -e 's/^typename //' >> $(PATH_CTAGS)
endef

define apply_clang_format
	@echo $(LOG) "Formateando archivo $(DIR_SRC)/$*.c con clang-format.." $(MSG_OK)
	@clang-format $1 --style=file -i
endef

define check_package_installed
ifeq (, $$(shell which $$1))
	$$(warning No tenes instalado el package $1 ejecuta `make install` e intentalo de nuevo)
endif
endef
