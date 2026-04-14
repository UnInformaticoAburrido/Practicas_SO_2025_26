#!/usr/bin/env bash

# Ejecuta el binario compilado mediante sh
sh -c "./programaC" &
PID=$!
# Espera a que termine
wait "$PID"
# Comprueba si el proceso sigue activo
if kill -0 "$PID" 2>/dev/null; then
    echo "El proceso sigue activo. Finalizando de forma controlada..."
    kill -TERM "$PID"
    sleep 1

    # Si sigue vivo, se fuerza el cierre
    if kill -0 "$PID" 2>/dev/null; then
        echo "El proceso no respondio. Forzando cierre..."
        kill -KILL "$PID"
    fi
else
    echo "El proceso finalizo correctamente."
fi