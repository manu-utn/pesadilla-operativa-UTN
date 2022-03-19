-include .config/Makefile.cfg
-include project.cfg
-include .config/functions.mk

##@ Entorno
i install: install-dev-utils install-lib-cspec install-lib-commons ## Instalar y configurar entorno (unica vez)

install-dev-utils:
	$(info Instalando utilidades de desarrollo...)
	@sudo apt install -y gcc gdb libcunit1 g++ libcunit1-dev \
  libncurses5 tig autotools-dev libfuse-dev libreadline6-dev \
	build-essential vagrant

install-lib-cspec:
	$(info Instalando cspec library...)
	@cd $(DIR_LIBS) && \
	sudo git clone http://github.com/mumuki/cspec
	@sudo $(MAKE) -C $(DIR_LIBS)/cspec clean all install

install-lib-commons:
	$(info Instalando so-commons...)
	@cd $(DIR_LIBS) && \
	sudo git clone http://github.com/sisoputnfrba/so-commons-library
	@sudo $(MAKE) -C $(DIR_LIBS)/so-commons-library clean all test install

##@ Desarrollo
compile: ## Compilar un módulo por su nombre (si no se especifíca el nombre, se compila el proyecto)
ifeq ($(COUNT_ARGS), 1)
	$(info Compilando todos los módulos dentro del contenedor...)
	@$(foreach modulo, $(DIR_MODULOS), \
			$(call specific_module_cmd,compile,$(modulo));)

else
	$(info Compilando un módulo...)
	@$(call module_cmd, compile)
endif

e exec: ## Ejecutar uno de los módulos
	$(info Ejecutando modulo...)
	@$(call module_cmd,compile exec)

tests: ## Ejecutar pruebas unitarias en un módulo
	@$(call module_cmd,tests)

##@ Extra
l list: ## Listar nombre de los módulos
	$(info $(DIR_MODULOS))

memcheck: ## Ejecutar Memcheck de Valgrind en un módulo
	$(info Ejecutando aplicación del contenedor...)
#	@$(call docker_make_cmd, memcheck)

##@ Utilidades
c clean: ## Remover ejecutables y logs de los modulos
	@$(foreach modulo, $(DIR_MODULOS), \
		$(call specific_module_cmd,clean,$(modulo));)

h help: ## Mostrar menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nOpciones para usar:\n  make \033[36m\033[0m\n"} /^[$$()% 0-9a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)
#	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

.PHONY: i install b build r run s stop e exec w watch h help c clean sh l list
