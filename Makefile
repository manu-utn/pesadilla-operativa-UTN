-include Makefile.cfg
-include project.cfg

b build: ## Construir aplicación
	$(info Construyendo imagen...)
	@docker build . -t $(CONTAINER) --build-arg DIR_PROJECT=$(DIR_PROJECT)

r run: ## Iniciar aplicación
	$(info Iniciando aplicación con Docker...)
	@docker run --detach -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH)/$(DIR_PROJECT):$(DIR_BASE)/$(DIR_PROJECT) \
		--user $(UID):$(GID) \
		$(CONTAINER)

s stop: ## Detener ejecución de la aplicación
	$(info Deteniendo contenedor...)
	@docker stop $(IMAGE_NAME)

sh: ## Acceder a la aplicación
	@docker exec -it $(IMAGE_NAME) /bin/sh

compile: ## Compilar un módulo por su nombre (si no se especifíca el nombre, se compila el proyecto)
ifeq ($(COUNT_ARGS), 1)
	$(info Compilando todos los módulos dentro del contenedor...)
	@$(foreach modulo, $(MODULOS), \
		docker exec $(IMAGE_NAME) make --no-print-directory -C $(modulo) compile;)
else
	$(info Compilando módulo dentro del contenedor...)
	@docker exec $(IMAGE_NAME) make --no-print-directory -C $(ARGS) compile
endif

e exec: ## Ejecutar un de los módulo
	$(info Ejecutando aplicación del contenedor...)
	@docker exec $(IMAGE_NAME) make --no-print-directory -C $(ARGS) compile exec

c clean: ## Remover ejecutables, objetos y dependencias
	@$(foreach modulo_dir, $(DIR_MODULOS), $(MAKE) --no-print-directory -C $(DIR_PROJECT)/$(modulo_dir) clean;)

tests: ## Ejecutar pruebas unitarias con CSpec en un módulo
	@docker exec $(IMAGE_NAME) make --no-print-directory -C $(ARGS) tests compile exec

memcheck: ## Ejecutar Memcheck en un módulo
	$(info Ejecutando aplicación del contenedor...)
	@docker exec $(IMAGE_NAME) make -C $(ARGS) memcheck

# TODO: Ya no es útil su queremos observar varios módulos (temporalmente)
w watch: ## (deprecado) Observar cambios en /src /include y compilar automáticamente
	@docker run -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH)/$(DIR_PROJECT):$(DIR_BASE)/$(DIR_PROJECT) \
		--user $(UID):$(GID)\
		$(CONTAINER)

h help: ## Muestra menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

.PHONY: b build r run s stop e exec w watch h help c clean sh
