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

# TODO: pendiente, problemas al hacer $(eval $(call apply_clang_format, nombrearchivo.c))
#
# define apply_clang_format
# ifneq (, $$(shell which clang-format))
# 	$$(info Formateando archivo $1 fuente con clang-format..)
# 	@clang-format $$1 --style=file -i
# endif
# endef
