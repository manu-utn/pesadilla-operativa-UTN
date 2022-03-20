-include .config/Makefile.cfg
-include project.cfg
-include .config/functions.mk

##@ Entorno
i install: install-virtualbox install-dev-utils install-lib-cspec install-lib-commons add-user copy-project ## Instalar y configurar entorno (unica vez)

copy-project:
ifeq ($(ENVIRONMENT_PROD), false)
	@sudo rsync -rvz . $(DIR_BASE)
	@sudo chown -R utnso:utnso $(DIR_BASE)
	@sudo chmod -R ug+rwx $(DIR_BASE)
endif

install-dev-utils:
	$(info Instalando utilidades de desarrollo...)
	@sudo apt install -y gcc gdb libcunit1 g++ libcunit1-dev \
  libncurses5 tig autotools-dev libfuse-dev libreadline6-dev \
	build-essential vagrant nemiver

install-virtualbox:
# Adding VirtualBox Package Repository:
	@echo "deb [arch=amd64] http://download.virtualbox.org/virtualbox/debian bionic contrib" | sudo tee /etc/apt/sources.list.d/virtualbox.list
# Adding VirtualBox Public PGP Key:
	@wget -q https://www.virtualbox.org/download/oracle_vbox_2016.asc -O- | sudo apt-key add -
	@sudo apt update && sudo apt install -y virtualbox virtualbox-ext-pack

add-user:
ifeq ($(ENVIRONMENT_PROD), false)
	$(info Configurando usuario utnso...)
# creamos el usuario, le asignamos una shell, un directorio y lo agregamos al grupo de sudo
	@sudo useradd -s /bin/bash -d $(DIR_BASE) -m -G sudo utnso
# le asignamos contraseña
	@sudo passwd utnso
# nos logeamos con ese usuario
	@su utnso && cd (DIR_BASE)
endif

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

d debug: ## Debugear uno de los módulos
	$(info Debugeando modulo...)
	@$(call module_cmd,debug)

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

.PHONY: i install b build r run s stop e exec w watch h help c clean l list install-virtualbox install-dev-utils install-lib-cspec install-lib-commons add-user copy-project
