-include Makefile.cfg

b build:
	$(info Construyendo contenedor...)
	@docker build . -t $(CONTAINER)

e exec:
	$(info Ejecutando aplicación del contenedor...)
	@docker exec $(IMAGE_NAME) make run

s stop:
	$(info Deteniendo contenedor...)
	@docker stop $(IMAGE_NAME)

w watch:
	@docker run -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH)/$(SHARED_FOLDER):/home/$(SHARED_FOLDER) \
		--user $(UID):$(GID) \
		$(CONTAINER)

.PHONY: b build e exec s stop w watch
