NAME_PUBLIC_KEY=id_rsa.pub
PATH_PUBLIC_KEY=~/.ssh/$(NAME_PUBLIC_KEY)
URL_UPLOAD_PUBLIC_KEY=https://transfer.sh

##@ Deploy
ssh-key-create: ## generar clave publica
	$(info Creando clave SSH..)
	@$(DIR_BASE)/.config/popup-ssh-keygen.sh

ssh-key-print: ## imprimir clave publica
	@echo "Agregar la siguiente clave publica en (https://github.com/settings/ssh/new)"
	@cat $(PATH_PUBLIC_KEY)
#	@cat ~/.ssh/id_rsa.pub

ssh-key-upload: ## subir clave publica a un servidor (https://transfer.sh)
	curl --upload-file $(PATH_PUBLIC_KEY) $(URL_UPLOAD_PUBLIC_KEY)/$(NAME_PUBLIC_KEY).txt
