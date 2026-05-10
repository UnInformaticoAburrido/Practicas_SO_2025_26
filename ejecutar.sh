#!/bin/sh

set -u

USAR_SERVIDOR=1
SERVER_PID=""

# Si se pasa -s, no se inicia el servidor simulado
if [ "${1:-}" = "-s" ]; then
    USAR_SERVIDOR=0
fi

# Iniciar servidor solo si no se ha usado -s
if [ "$USAR_SERVIDOR" -eq 1 ]; then
    echo "Iniciando servidor simulado..."
    python3 ServidorCamara.py &
    SERVER_PID=$!

    sleep 1
else
    echo "Modo sin servidor simulado."
fi

# Ejecutar cliente y esperar hasta que termine
echo "Ejecutando cliente..."
./cliente

# Si se inició servidor simulado, cerrarlo con cadena de señales
if [ "$USAR_SERVIDOR" -eq 1 ]; then
    echo "Enviando SIGTERM al servidor..."
    kill -TERM "$SERVER_PID" 2>/dev/null || true

    sleep 4

    if kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "Servidor sigue activo. Enviando SIGINT..."
        kill -INT "$SERVER_PID" 2>/dev/null || true

        sleep 4

        if kill -0 "$SERVER_PID" 2>/dev/null; then
            echo "Servidor sigue activo. Enviando SIGKILL..."
            kill -KILL "$SERVER_PID" 2>/dev/null || true
        fi
    fi
fi

echo "Ejecución finalizada."