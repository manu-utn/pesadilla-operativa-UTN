-include .config/Makefile.cfg
-include .config/message-colors.mk
-include .config/functions.mk
-include .config/docker.mk
-include .config/install.mk
-include .config/deploy.mk
-include .config/packages-installed.mk
-include project.cfg

##@ Entorno
i install: install-virtualbox install-dev-utils install-ctags install-lib-cspec install-lib-commons add-user copy-project ## Instalar y configurar entorno (unica vez)
	echo "#define DIR_BASE \"/home/$(USERNAME)/tp-2022-1c-Sisop-Oh-Yeah/project/\"" > ./project/static/include/dir.h

##@ Desarrollo
# TODO: need refactor
compile: ctags-installed libcommons-installed ## Compilar un módulo por su nombre (si no se especifíca el nombre, se compila el proyecto)
ifeq ($(COUNT_ARGS), 1)
	$(info Compilando todos los módulos dentro del contenedor...)
	@$(foreach modulo, $(DIR_MODULOS), \
			$(call specific_module_cmd,compile,$(modulo)) 2>&1 | tee -a logs/compilation.log;)
else
	$(info Compilando un módulo...)
	@$(call module_cmd, compile) 2>&1 | tee -a logs/compilation.log
	@$(call create_ctags,$(SOURCES))
endif

e exec: ## Ejecutar uno de los módulos
	$(info Ejecutando modulo...)
	$(call module_cmd,compile exec)

CONSOLA_ARCHIVO=instrucciones.txt
ARCHIVO?=$(DIR_BASE)/$(DIR_PROJECT)/consola/config/$(CONSOLA_ARCHIVO)
TAMANIO?=500

exec-consola:
	$(info Ejecutando modulo consola...)
	$(MAKE) --no-print-directory -C $(DIR_PROJECT)/consola compile
	$(DIR_PROJECT)/consola/bin$(DIR_BIN)/consola.out $(ARCHIVO) $(TAMANIO)

debug-consola:
	$(info Ejecutando modulo consola...)
	$(MAKE) --no-print-directory -C $(DIR_PROJECT)/consola compile
	$(DEBUGGER) $(DIR_PROJECT)/consola/bin$(DIR_BIN)/consola.out $(ARCHIVO) $(TAMANIO)

d debug: debugger-installed ## Debugear uno de los módulos
	$(info Debugeando modulo...)
	@$(call module_cmd,debug)

t test: libcspecs-installed ## Ejecutar pruebas unitarias en un módulo
	@$(call module_cmd,test)

memcheck: valgreen-installed ## Ejecutar Memcheck de Valgrind en un módulo
	$(info Ejecutando memcheck de valgrind en un modulo...)
	@$(call module_cmd, memcheck)

##@ Extra

# - el parametro -f nos cambiar el contexto que usa docker, asignandole el directorio actual
# - docker por defecto usa como contexto la ruta donde esta el Dockerfile,
# siendo  esta su ruta relativa, no pudiendo usar COPY .. . es decir no copiaria la ruta padre
simulation: docker-installed ## Simulacion en un Servidor Ubuntu 14.0 (interaccion solo por terminal)
	$(info Iniciando simulacion en Ubuntu 14.0...)
	@docker build -f .config/Dockerfile . -t $(CONTAINER)
	@docker run -it --rm --name $(IMAGE_NAME) \
		-v $(CURRENT_PATH):/home/utnso/tp \
		$(CONTAINER)
# --user $(UID):$(GID) \

w watch: screen-installed ## Observar cambios y compilar automaticamente todos los modulos
	$(info Observando cambios en la aplicación...)
	@$(foreach modulo, $(DIR_MODULOS), \
		screen -dmS $(modulo) && \
		screen -S $(modulo) -X stuff "make --no-print-directory -C project/$(modulo) watch 1>logs/compilation.log 2>&1\n";)
#		screen -S $(modulo) -X stuff "make --no-print-directory -C project/$(modulo) watch 1>logs/compilation.log 2>/dev/null\n";)
	@$(DIR_BASE)/.config/popup-confirm-stopwatch.sh

stopwatch: ## Dejar de observar cambios
	@$(DIR_BASE)/.config/popup-confirm-stopwatch.sh

logs: lnav-installed ## Ver logs de compilacion
ifneq ("", "$(wildcard $(DIR_COMPILE_LOGS)/compilation.log)")
	@lnav $(DIR_COMPILE_LOGS)/compilation.log
else
	$(error No se crearon logs de compilacion aun)
endif

##@ Utilidades
clean: clean-logs ## Remover ejecutables y logs de compilacion
	@$(foreach modulo, $(DIR_MODULOS), $(call specific_module_cmd,clean,$(modulo));)
	@$(foreach lib, $(DIR_LIBRARIES), $(call specific_module_cmd,clean,$(lib));)

clean-logs:
	@echo "Removiendo logs de compilacion"
	@-$(RM) $(DIR_COMPILE_LOGS)/*.log

h help: ## Mostrar menú de ayuda
	@awk 'BEGIN {FS = ":.*##"; printf "\nOpciones para usar:\n  make \033[36m\033[0m\n"} /^[$$()% 0-9a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)
#	@awk 'BEGIN {FS = ":.*##"; printf "\nGuía de Comandos:\n  make \033[36m\033[0m\n"} /^[$$()% a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

# necesario para evitar
# el warning hubiera sido "make: *** No rule to make target 'nombre'.  Stop."
%:
	@true

.PHONY: i install b build r run s stop e exec w watch stopwatch h help t test simulation logs logs-error clean-logs
