#!/bin/bash
read -p "Estas seguro? (si/no)" choice
case "$choice" in
    si ) echo "si";;
    * ) echo no && exit 1
esac
