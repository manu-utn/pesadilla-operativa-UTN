#!/bin/bash
CMD_STOPWATCH="\033[32mmake stopwatch\033[0m"
CMD_LOGS="\033[32mmake logs\033[0m"

read -p "Quieres detener ahora el watch? (si/no) " choice
case "$choice" in
    si ) pkill screen ;;
    * ) printf "Si luego quiere detenerlo escriba ${CMD_STOPWATCH} o ${CMD_LOGS} para ver los logs\n"
esac
