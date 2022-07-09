#!/bin/bash

FILE=~/.ssh/id_rsa

if [[ -f "$FILE" ]]; then
    echo "Error! La clave ssh ya esta generada en el equipo.."
    exit 1
else
    read -p "Ingrese su correo asociado a github: " email
    # si usamos otro nombre de clave en vez de "id_rsa", tendremos problemas de permisos
    # con el agente de ssh y tendremos que agregarlo
    ssh-keygen -t ed25519 -C "$email" -f ~/.ssh/id_rsa -q -N ""
fi
