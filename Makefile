-include Makefile.cfg

b build: ## Construir imagen de Docker
	$(info Construyendo imagen...)
	@docker build . -t $(CONTAINER)

e exec: ## Ejecutar aplicación dentro del contenedor Docker
	$(info Ejecutando aplicación del contenedor...)
	@docker exec $(IMAGE_NAME) make exec

memcheck: ## Ejecutar Memcheck con la aplicación dentro del contenedor Docker
	$(info Ejecutando aplicación del contenedor...)
	@docker exec $(IMAGE_NAME) make memcheck

s stop: ## Detener ejecución del contenedor de Docker
	$(info Deteniendo contenedor...)
	@docker stop $(IMAGE_NAME)

w watch: ## Observar cambios en /src /include y compilar automáticamente
	@docker run -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH)/$(SHARED_FOLDER):/home/$(SHARED_FOLDER) \
		--user $(UID):$(GID) \
		$(CONTAINER)

# TODO: la ruta para el clean cambiará cuando haya varios módulos
c clean:
	@$(MAKE) --no-print-directory -C data clean

h help: ## Muestra menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)


.PHONY: b build e exec s stop w watch h help c clean
