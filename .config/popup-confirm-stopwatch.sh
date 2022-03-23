#!/bin/bash
read -p "Quieres detener ahora el watch? (si/no) " choice
case "$choice" in
    si ) pkill screen ;;
    * ) echo "Si luego quiere detenerlo escriba make stopwatch"
esac
