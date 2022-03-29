export DOCKER_RUNNING

# TODO: need refactor, se requiere una funcion que lo haga mas generico (ojo, se debe usar $$ si queremos usar ifeq en un define)
lnav-installed:
ifeq (, $(shell which lnav))
	$(error No tenes instalado el package clang-format ejecuta `make install` e intentalo de nuevo)
endif

ctags-installed:
ifeq (, $(shell which ctags))
	$(error No tenes instalado el package universal-ctags ejecuta `make install` e intentalo de nuevo)
endif

valgreen-installed:
ifeq (, $(shell which valgrind))
	$(error No tenes instalado el package valgrind ejecuta `make install` e intentalo de nuevo)
endif
ifeq (, $(shell which valgreen))
	$(error No tenes instalado el package valgreen ejecuta `make install` e intentalo de nuevo)
endif

debugger-installed:
ifeq ($(DOCKER_RUNNING), true)
	$(error el debugger no puede ser ejecutado sobre la simulacion, porque no tiene entorno grafico)
endif
ifeq (, $(shell which nemiver))
	$(error No tenes instalado el package nemiver ejecuta `make install` e intentalo de nuevo)
endif


screen-installed:
ifeq (, $(shell which screen))
	$(error No tenes instalado el package screen ejecuta `make install` e intentalo de nuevo)
endif

docker-installed:
ifeq (, $(shell which docker))
	$(error No tenes instalado el package docker ejecuta `make install` e intentalo de nuevo)
endif

libcommons-installed:
ifneq ($(DOCKER_RUNNING), true)
ifeq (, $(shell ldconfig -p | grep libcommons))
	$(error No tenes instalado la shared library so-commons ejecuta `make install` e intentalo de nuevo)
endif
endif

libcspecs-installed:
ifneq ($(DOCKER_RUNNING), true)
ifeq (, $(shell ldconfig -p | grep libcspec))
	$(error No tenes instalado la shared cspecs ejecuta `make install` e intentalo de nuevo)
endif
endif

.PHONY: lnav-installed ctags-installed valgreen-installed debugger-installed screen-installed docker-installed libcommons-installed libcspecs-installed
