# Practica 4 - Procesamiento e Interpolacion de Imagenes Tactiles

Proyecto de Sistemas Operativos desarrollado en C++ y Python para procesar capturas tactiles de un gripper robotico. El cliente C++ lee capturas `16x16` desde un archivo JSON, valida sus dimensiones, aplica interpolacion bilineal manual hasta obtener matrices `128x128` y envia cada captura interpolada por HTTP POST a un servidor Python. El servidor recibe los datos y genera imagenes PNG con Matplotlib.

## Objetivo

El objetivo de la practica es construir un sistema distribuido cliente-servidor capaz de:

- leer datos tactiles almacenados en JSON;
- validar matrices tactiles de `16x16`;
- aumentar la resolucion hasta `128x128`;
- enviar las matrices interpoladas mediante HTTP POST;
- generar representaciones visuales de presion tactil.

## Arquitectura

```text
tactile_captures_50.json
        |
        v
Cliente C++: main.cpp + libreria.cpp
        |
        | HTTP POST /captura
        v
Servidor Python Flask: servidor.py
        |
        v
salida/capture_0.png ... capture_49.png
```

El cliente realiza el procesamiento numerico. El servidor se encarga de la recepcion HTTP y de la visualizacion.

## Flujo del Cliente C++

El archivo `main.cpp` mantiene el flujo principal visible:

1. lee `tactile_captures_50.json`;
2. valida que todas las capturas sean matrices `16x16`;
3. interpola cada matriz con interpolacion bilineal manual;
4. envia cada matriz `128x128` al servidor Python;
5. muestra mensajes de error o exito por consola.

## Interpolacion Bilineal

La funcion `interpolar_bilineal()` escala cada matriz desde `16x16` hasta `128x128` usando factor `8`.

Para cada posicion nueva se calcula su posicion equivalente dentro de la matriz original y se combinan los cuatro vecinos mas cercanos:

- superior izquierdo;
- superior derecho;
- inferior izquierdo;
- inferior derecho.

No se usa OpenCV, SciPy ni ninguna funcion automatica de escalado.

## Comunicacion HTTP

El cliente envia cada captura interpolada como JSON mediante `libcurl`.

Formato enviado:

```json
{
  "capture_id": 0,
  "width": 128,
  "height": 128,
  "data": [[0.0, 1.0, 2.0]]
}
```

El servidor Flask recibe la peticion en:

```text
POST /captura
```

Si la matriz recibida es valida, genera una imagen PNG llamada:

```text
capture_{id}.png
```

## Dependencias

### C++
- JSON for Modern C++ (`nlohmann/json`).
- `libcurl` para realizar peticiones HTTP POST.

### Python

- Flask;
- NumPy;
- Matplotlib.

## Linux
1. ```chmod +x install_deps.sh```
2. ```./install_deps.sh```

## Windows
1. ```install_deps.bat```

## Compilacion

# Linux
```bash
g++ main.cpp libreria.cpp -std=c++17 -lcurl -o cliente
```
# Windows
```bash
g++ main.cpp libreria.cpp -std=c++17 -o main.exe -lcurl
```

El proyecto incluye `nlohmann/json.hpp`, por lo que no hace falta instalar esa libreria aparte si el compilador encuentra la carpeta del proyecto.

## Ejecucion

Primero arrancar el servidor:

```bash
python servidor.py
```

Despues ejecutar el cliente:

```bash
./cliente
```

En Windows, si se compilo como `cliente_windows.exe`:

```bat
cliente_windows.exe
```

## Configuracion

Configuracion del cliente en `libreria.h`:

```cpp
const std::string rutaArchivoJson = "tactile_captures_50.json";
const std::string servidor = "127.0.0.1";
const int puerto = 5000;
const std::string ruta = "/captura";
const int factorEscala = 8;
```

Configuracion del servidor en `config_servidor.json`:

```json
{
  "host": "127.0.0.1",
  "puerto": 5000,
  "carpetaSalida": "salida"
}
```

## Resultados Esperados

Al ejecutar correctamente el sistema:

- el cliente carga `50` capturas;
- cada captura se valida como matriz `16x16`;
- cada matriz se interpola a `128x128`;
- cada captura se envia al servidor;
- el servidor genera `50` imagenes PNG en `salida/`.

Ejemplo de salida del cliente:

```text
Capturas cargadas correctamente: 50
Captura 0 enviada correctamente.
...
Captura 49 enviada correctamente.
Proceso completado correctamente.
```

## Problemas y Consideraciones

- El cliente necesita que el servidor Python este ejecutandose antes de enviar datos.
- Si `libcurl` no esta instalado, el cliente C++ no compilara.
- Si el JSON no contiene `captures[].matrix`, el programa termina con error.
- Si alguna matriz no es `16x16`, el programa termina con error.
- El servidor usa Matplotlib en modo `Agg` para poder generar imagenes sin abrir ventanas.

### Error de DLLs en Windows

En Windows puede aparecer un error al ejecutar `main.exe` aunque la compilacion haya terminado correctamente. Un ejemplo comun es un mensaje indicando que no se encuentra un punto de entrada en una DLL relacionada con `libssl`, `libcrypto`, `libcurl` o `libngtcp2`.

Este problema normalmente no esta causado por el codigo del proyecto, sino por un conflicto entre librerias instaladas en el sistema. Por ejemplo, si hay varias versiones de OpenSSL o libcurl en el `PATH`, Windows puede cargar una DLL incompatible antes que la usada por el compilador.

Solucion rapida si se compilo con MSYS2 UCRT64:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
.\main.exe
```

Tambien se puede ejecutar el programa desde una terminal MSYS2 UCRT64, donde esas librerias ya suelen tener prioridad.

## Cumplimiento de la Practica

Este proyecto cumple los puntos principales indicados:

- lectura de JSON en C++;
- validacion de matrices;
- interpolacion bilineal manual;
- comunicacion HTTP POST;
- servidor Flask en Python;
- generacion de imagenes con Matplotlib;
- organizacion mediante funciones;
- separacion entre cliente C++ y servidor Python.
