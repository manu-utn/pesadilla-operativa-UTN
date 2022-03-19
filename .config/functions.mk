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
