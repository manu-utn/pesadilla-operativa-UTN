-include Makefile.cfg
-include project.cfg

docker-build: ## Construir imagen de Docker
	$(info Construyendo imagen...)
	@docker build . -t $(CONTAINER)

docker-run: ## Iniciar aplicación con Docker
	$(info Iniciando aplicación con Docker...)
	@docker run --detach -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH)/$(SHARED_FOLDER):/home/$(SHARED_FOLDER) \
		--user $(UID):$(GID) \
		$(CONTAINER)

s stop: ## Detener ejecución del contenedor de Docker
	$(info Deteniendo contenedor...)
	@docker stop $(IMAGE_NAME)

b build: ## Compilar uno de los módulos
	$(info Compilando módulo dentro del contenedor...)
	@docker exec $(IMAGE_NAME) make --no-print-directory -C $(ARGS) build

build-all: ## Compilar todos los módulos
	$(info Compilando todos los módulos dentro del contenedor...)
	@$(foreach modulo, $(MODULOS), \
		docker exec $(IMAGE_NAME) make --no-print-directory -C $(modulo) build;)

e exec: ## Ejecutar uno de los módulos
	$(info Ejecutando aplicación del contenedor...)
	@docker exec $(IMAGE_NAME) make --no-print-directory -C $(ARGS) build exec

tests: ## Ejecutar pruebas unitarias con CSpec en un módulo
	@docker exec $(IMAGE_NAME) make --no-print-directory -C $(ARGS) tests build exec

memcheck: ## Ejecutar Memcheck en un módulo
	$(info Ejecutando aplicación del contenedor...)
	@docker exec $(IMAGE_NAME) make -C $(ARGS) memcheck

# TODO: Ya no es útil su queremos observar varios módulos (temporalmente)
w watch: ## (deprecado) Observar cambios en /src /include y compilar automáticamente
	@docker run -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH)/$(SHARED_FOLDER):/home/$(SHARED_FOLDER) \
		--user $(UID):$(GID) \
		$(CONTAINER)

c clean:
	@$(foreach modulo, $(MODULOS), $(MAKE) --no-print-directory -C data/$(modulo) clean;)

h help: ## Muestra menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)


.PHONY: b build r run e exec s stop w watch h help c clean build-all docker-build docker-run
