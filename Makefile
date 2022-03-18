-include .config/Makefile.cfg
-include project.cfg
-include .config/functions.mk

##@ Entorno
# https://docs.docker.com/engine/install/ubuntu/
preinstall-docker:
# 1. install Docker dependencies
	@sudo apt install -y apt-transport-https ca-certificates curl gnupg-agent software-properties-common
# 2. add Docker’s official GPG key which is important in enabling Docker repo
	@curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
# 3. add Docker repository
	@echo "deb [arch=amd64] https://download.docker.com/linux/ubuntu focal stable" | sudo tee /etc/apt/sources.list.d/docker.list

setup: preinstall-docker ## Instalar y configurar entorno (unica vez)
	$(info Configurando entorno...)
# 1. Install Docker Engine
	@sudo apt update
	@sudo apt install -y docker-ce docker-ce-cli containerd.io
# 2. Agregamos al usuario actual al grupo `docker` para que pueda ejecutar los comandos con docker
	@sudo usermod -aG docker $(WHOAMI)
# 3. Con `newgrp` habilitamos el cambio de grupo, sin necesidad de hacer `logout`ni `reboot`
	@newgrp docker

b build: ## Construir aplicación (unica vez)
	$(info Construyendo imagen...)
	@docker build .config -t $(CONTAINER) --build-arg DIR_PROJECT=$(DIR_PROJECT)

r run: ## Iniciar aplicación
	$(info Iniciando aplicación con Docker...)
	@docker run --detach -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH)/$(DIR_PROJECT):$(DIR_BASE)/$(DIR_PROJECT) \
		--user $(UID):$(GID) \
		$(CONTAINER)
s stop: ## Detener ejecución de la aplicación
	$(info Deteniendo contenedor...)
	@$(call docker_cmd, stop)

##@ Desarrollo
compile: ## Compilar un módulo por su nombre (si no se especifíca el nombre, se compila el proyecto)
ifeq ($(COUNT_ARGS), 1)
	$(info Compilando todos los módulos dentro del contenedor...)
	@$(foreach modulo, $(DIR_MODULOS), \
		 $(call docker_make_cmd, $(modulo) compile);)
else
	$(info Compilando módulo dentro del contenedor...)
	@$(call docker_make_cmd, compile)
endif

e exec: ## Ejecutar uno de los módulos
	$(info Ejecutando aplicación del contenedor...)
	@$(call docker_make_cmd, compile exec)

tests: ## Ejecutar pruebas unitarias en un módulo
	$(call docker_make_cmd, tests)

##@ Extra
sh: ## Acceder a la aplicación por terminal
	$(call docker_cmd, exec -it, /bin/sh)

l list: ## Listar nombre de los módulos
	$(info $(DIR_MODULOS))

memcheck: ## Ejecutar Memcheck de Valgrind en un módulo
	$(info Ejecutando aplicación del contenedor...)
	@$(call docker_make_cmd, memcheck)

# TODO: Ya no es útil su queremos observar varios módulos (temporalmente)
w watch: # (deprecado) Observar cambios en /src /include y compilar automáticamente
	@docker run -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH)/$(DIR_PROJECT):$(DIR_BASE)/$(DIR_PROJECT) \
		--user $(UID):$(GID)\
		$(CONTAINER)

##@ Utilidades
c clean: ## Remover ejecutables y logs de los modulos
	@$(foreach modulo_dir, $(DIR_MODULOS), \
		$(call make_cmd, $(DIR_PROJECT)/$(modulo_dir) clean);)

h help: ## Mostrar menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nOpciones para usar:\n  make \033[36m\033[0m\n"} /^[$$()% 0-9a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)
#	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

.PHONY: b build r run s stop e exec w watch h help c clean sh l list
