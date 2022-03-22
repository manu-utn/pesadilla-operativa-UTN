# TODO: pendiente definir
deploy-dev: popup-confirm-action
#	$(info Haciendo deploy)

deploy-prod: popup-confirm-action ## Deploy a servidor remoto
	$(info Subiendo proyecto a servidor remoto...)
	@rsync -a --progress --partial --rsh=ssh . $(SSH_USER)@$(SSH_IP):$(SSH_PATH_DEST)

popup-confirm-action:
	@.config/popup-confirm-action.sh

copy-project:
ifeq ($(USER_UTNSO_IS_REQUIRED), false)
	@sudo rsync -rvz . $(DIR_BASE)
	@sudo chown -R utnso:utnso $(DIR_BASE)
	@sudo chmod -R ug+rwx $(DIR_BASE)
endif

install-dev-utils:
	$(info Instalando utilidades de desarrollo...)
	@-sudo apt install -y gcc gdb libcunit1 g++ libcunit1-dev \
  libncurses5 tig autotools-dev libfuse-dev libreadline6-dev \
	build-essential vagrant
	@-sudo apt install -y nemiver rsync
	@-sudo apt install -y clang-format
	@-sudo apt install -y universal-ctags

install-virtualbox:
ifeq ($(VBOX_IS_REQUIRED), true)
ifeq ($(VBOX_LATEST), true)
# Adding VirtualBox Package Repository:
	@echo "deb [arch=amd64] http://download.virtualbox.org/virtualbox/debian bionic contrib" | sudo tee /etc/apt/sources.list.d/virtualbox.list
# Adding VirtualBox Public PGP Key:
	@wget -q https://www.virtualbox.org/download/oracle_vbox_2016.asc -O- | sudo apt-key add -
	@sudo apt update && sudo apt install -y virtualbox virtualbox-ext-pack
else
	@cd /tmp && \
	wget https://download.virtualbox.org/virtualbox/6.0.24/virtualbox-6.0_6.0.24-139119~Ubuntu~bionic_amd64.deb && \
	sudo dpkg -i virtualbox-6.0_6.0.24-139119~Ubuntu~bionic_amd64.deb && \
	sudo rm -vf virtualbox-6.0_6.0.24-139119~Ubuntu~bionic_amd64.deb
endif
endif

add-user:
ifeq ($(USER_UTNSO_IS_REQUIRED), true)
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

install-ctags:
ifneq (, $(shell which universal-ctags))
	$(info Instalando ctags...)
	@cd /tmp && \
			git clone https://github.com/universal-ctags/ctags.git && cd ctags && \
    ./autogen.sh && ./configure && make && sudo make install
endif

.PHONY: install-virtualbox install-dev-utils install-ctags install-lib-cspec install-lib-commons add-user copy-project deploy-dev deploy-prod
