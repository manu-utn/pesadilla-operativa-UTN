define make_cmd
	make  --no-print-directory -C $(ARGS) $1
endef

define docker_cmd
	@docker $1 $(IMAGE_NAME) $2
endef

define docker_make_cmd
	@docker exec $(IMAGE_NAME) \
		make --no-print-directory -C $(ARGS) $1
endef
