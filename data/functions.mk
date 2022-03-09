define make_exec
	@$(MAKE) --no-print-directory -f $1 $2
endef

define watch
	@while true; do $(MAKE) -q || $(MAKE) --no-print-directory; sleep 1; done
endef
