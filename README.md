<<<<<<< HEAD
# Practica1 - Captura y visualización de datos MQTT

Este repositorio permite:

1. **Suscribirse a un broker MQTT** usando el ejecutable `mqtt_subscribe_emqx_linux`.
2. **Capturar mensajes** en un archivo de log (`mqtt_capture.log`).
3. **Extraer payloads JSON numéricos** desde el log.
4. **Generar dos visualizaciones**:
   - Una imagen con Matplotlib (por defecto `Grafica.png`).
   - Un gráfico ASCII en consola.

---

## Estructura principal

- `capture_mqtt.sh`: automatiza la captura, detiene el proceso de suscripción y ejecuta el análisis/graficación en Python.
- `plot_mqtt.py`: script Python para leer el log, extraer los datos y generar gráficos.
- `mqtt_subscribe_emqx_linux`: ejecutable de suscripción MQTT.
- `Extras/InstaladorMQTT.sh`: instalador automático para dependencias de Paho MQTT en Linux cuando hay incompatibilidades de versión.

---

## Dependencias necesarias

### Sistema operativo
- Linux (el flujo principal está pensado para Bash/Linux).

### Herramientas base
- `bash`
- `python3`
- `pip` (recomendado para instalar librerías Python)
#### Librerías de Python
- `matplotlib`

Instalación recomendada:

```bash
python3 -m pip install matplotlib
```

> Nota: el binario `mqtt_subscribe_emqx_linux` depende de librerías MQTT Paho C/C++. Si hay problemas por versiones, revisa la sección de instalador automático más abajo.

---

## ¿Cómo funciona la aplicación?

### Opción A (flujo automático)
Ejecuta:

```bash
chmod +x ./capture_mqtt.sh
./capture_mqtt.sh
```

Flujo interno:
1. Comprovamos dependencias locales
2. Pide por teclado el tiempo de captura (segundos).
3. Lanza `mqtt_subscribe_emqx_linux` y redirige su salida a `mqtt_capture.log`.
4. Espera el tiempo indicado y detiene el proceso.
5. Ejecuta la lógica Python que:
   - Lee el log.
   - Busca líneas con `Payload:`.
   - Intenta parsear objetos JSON.
   - Extrae valores numéricos.
   - Genera `Grafica.png` y un gráfico ASCII en terminal.

### Opción B (solo procesado Python)
Si ya tienes un log generado, puedes correr directamente:

```bash
python3 plot_mqtt.py
```

---

## Configuración en Python: 3 constantes editables

En la parte de Python hay **tres constantes** que puedes modificar para personalizar el comportamiento:

```python
LOG_FILE = "mqtt_capture.log"
OUTPUT_IMAGE = "Grafica.png"
PRECISION_GRAFICA = 10
```

- `LOG_FILE`: archivo desde donde se extrae la información.
- `OUTPUT_IMAGE`: nombre del archivo de salida de la gráfica.
- `PRECISION_GRAFICA`: precisión/altura del gráfico ASCII en consola.

Puedes cambiar estas constantes en '''    python3 - << 'PY'
import sys
import json
import matplotlib.pyplot as plt'''(y también aparecen en el bloque Python embebido dentro de `plot_mqtt.py`).

---

## Instalador automático para errores de versión MQTT Paho

Si la versión de MQTT Paho instalada no es la correcta o faltan componentes, el repositorio incluye un instalador automático:

```bash
bash chmod +x Extras/InstaladorMQTT.sh
bash Extras/InstaladorMQTT.sh
```

Este script intenta instalar/reinstalar dependencias de Paho MQTT en Linux para dejar un entorno compatible.

---

## Salidas esperadas

- `mqtt_capture.log`: log de mensajes MQTT capturados.
- `Grafica.png` (o el nombre que configures): gráfica de los valores numéricos encontrados.
- Gráfico ASCII en terminal.

---

## Licencia

Revisar el archivo `LICENSE`.

## Herramientas externas empleadas
- [Regexr](https://regexr.com/)
Generacion y examinacion de regex.
=======
# Practicas_SO_2025_26
>>>>>>> eeae0e2ea09378d507036bb948ac1052d5b49f17
