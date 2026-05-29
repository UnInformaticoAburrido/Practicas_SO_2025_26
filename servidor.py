import json
import os

import matplotlib
matplotlib.use("Agg")

from flask import Flask, jsonify, request
import matplotlib.pyplot as plt
import numpy as np


def leer_config_servidor(ruta_config):
    valores_por_defecto = {
        "host": "127.0.0.1",
        "puerto": 5000,
        "carpetaSalida": "salida",
    }

    if not os.path.exists(ruta_config):
        return valores_por_defecto

    with open(ruta_config, "r", encoding="utf-8") as archivo:
        config = json.load(archivo)

    valores_por_defecto["host"] = config.get("host", valores_por_defecto["host"])
    valores_por_defecto["puerto"] = int(config.get("puerto", valores_por_defecto["puerto"]))
    valores_por_defecto["carpetaSalida"] = config.get("carpetaSalida", valores_por_defecto["carpetaSalida"])
    return valores_por_defecto


configuracion = leer_config_servidor("config_servidor.json")
os.makedirs(configuracion["carpetaSalida"], exist_ok=True)

app = Flask(__name__)


def validar_peticion(datos):
    campos = ["capture_id", "width", "height", "data"]
    for campo in campos:
        if campo not in datos:
            return False, f"Falta el campo {campo}"

    matriz = datos["data"]
    alto = datos["height"]
    ancho = datos["width"]

    if not isinstance(matriz, list) or len(matriz) != alto:
        return False, "La altura no coincide con data"

    for fila in matriz:
        if not isinstance(fila, list) or len(fila) != ancho:
            return False, "El ancho no coincide con data"

    return True, ""


@app.route("/captura", methods=["POST"])
def recibir_captura():
    datos = request.get_json(force=True, silent=True)
    if datos is None:
        return jsonify({"error": "JSON invalido"}), 400

    es_valida, mensaje = validar_peticion(datos)
    if not es_valida:
        return jsonify({"error": mensaje}), 400

    capture_id = int(datos["capture_id"])
    matriz = np.array(datos["data"], dtype=float)

    ruta_salida = os.path.join(configuracion["carpetaSalida"], f"capture_{capture_id}.png")

    plt.figure(figsize=(6, 5))
    plt.imshow(matriz, cmap="inferno")
    plt.colorbar(label="Presion")
    plt.title(f"Captura tactil {capture_id}")
    plt.tight_layout()
    plt.savefig(ruta_salida)
    plt.close()

    return jsonify({"status": "ok", "archivo": ruta_salida}), 201


if __name__ == "__main__":
    app.run(host=configuracion["host"], port=configuracion["puerto"])
