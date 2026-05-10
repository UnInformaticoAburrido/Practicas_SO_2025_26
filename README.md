# Práctica 3: Comunicación Cliente-Servidor con Sockets TCP/IP en C++

Este proyecto implementa una comunicación básica **cliente-servidor** usando sockets TCP/IP.

El servidor está escrito en **Python** y simula un dispositivo remoto, como una cámara o sensor, que envía datos binarios al cliente. El cliente está escrito en **C++**, se conecta al servidor, recibe los datos, los interpreta como valores `float`, los guarda en un archivo de texto y los muestra por pantalla.

Dicho en cristiano: un programa manda bytes, el otro los recibe y los trata como números. El ordenador no hace magia, solo obedece órdenes con una confianza preocupante.

---

## Objetivos del proyecto

- Crear un socket TCP/IP en C++.
- Conectar un cliente C++ con un servidor Python.
- Recibir datos mediante `recv()`.
- Guardar los datos recibidos en un `std::vector<float>`.
- Escribir los resultados en un archivo `.txt`.
- Mostrar los datos por consola.
- Separar el código en varios archivos para mejorar su organización.
- Automatizar la ejecución mediante un script `sh`.

---

## Arquitectura general

```text
Servidor Python
      ↓
Socket TCP/IP
      ↓
Cliente C++
      ↓
Vector en memoria
      ↓
datos.txt
```

El flujo del programa es el siguiente:

1. `ServidorCamara.py` abre un socket TCP en `127.0.0.1:12345`.
2. El cliente C++ crea su socket.
3. El cliente configura la IP y el puerto del servidor.
4. El cliente se conecta al servidor.
5. El servidor envía un bloque de datos binarios.
6. El cliente recibe esos datos usando `recv()`.
7. Los datos se almacenan en un `std::vector<float>`.
8. El cliente guarda los valores en `datos.txt`.
9. El cliente imprime los valores por pantalla.
10. El script `ejecutar.sh` cierra el servidor simulado si lo había iniciado.

---

## Estructura de archivos

| Archivo | Función |
|---|---|
| `cliente.cpp` | Contiene la función `main()` y organiza el flujo principal del cliente. |
| `libreria.h` | Declara las funciones usadas por el cliente. |
| `libreria.cpp` | Implementa las funciones de creación de socket, conexión, recepción, guardado e impresión. |
| `config.h` | Define la IP, el puerto y la ruta del archivo de salida. |
| `ServidorCamara.py` | Servidor simulado que envía datos binarios al cliente. |
| `ejecutar.sh` | Script para lanzar el servidor simulado y ejecutar el cliente. |
| `datos.txt` | Archivo generado con los datos recibidos. |

---

## Configuración

La configuración principal está en `config.h`:

```cpp
#define IP "127.0.0.1"
#define PORT 12345
#define FILE_PATH "datos.txt"
```

- `IP`: dirección del servidor.
- `PORT`: puerto TCP usado para la conexión.
- `FILE_PATH`: archivo donde se guardan los datos recibidos.

Por defecto se usa `127.0.0.1`, es decir, el propio ordenador.

---

## Funcionamiento del servidor Python

El archivo `ServidorCamara.py` crea un servidor TCP sencillo:

```python
HOST = '127.0.0.1'
PORT = 12345
```

Después genera un array de 768 elementos:

```python
data_array = [i % 256 for i in range(768)]
```

Ese array se empaqueta como bytes con:

```python
data_bytes = struct.pack('768B', *data_array)
```

Finalmente, cuando el cliente se conecta, el servidor envía todos los bytes mediante:

```python
conn.sendall(data_bytes)
```

---

## Funcionamiento del cliente C++

El cliente sigue este proceso en `main()`:

```cpp
int sockfd = CrearSocket();
sockaddr_in sockaddr = CrearDireccionServidor();
ConectarServidor(sockfd, sockaddr);
std::vector<float> floats = RecibirDatos(sockfd);
Guardar(floats);
imprimir(floats);
close(sockfd);
```

### 1. Crear el socket

La función `CrearSocket()` crea un socket TCP/IP:

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

- `AF_INET`: usa IPv4.
- `SOCK_STREAM`: usa TCP.
- `0`: protocolo por defecto para TCP.

Si falla, devuelve `-1`.

### 2. Configurar dirección del servidor

La función `CrearDireccionServidor()` prepara la estructura `sockaddr_in` con la IP y el puerto definidos en `config.h`.

```cpp
servaddr_out.sin_family = AF_INET;
servaddr_out.sin_port = htons(PORT);
inet_pton(AF_INET, IP, &servaddr_out.sin_addr);
```

`htons()` convierte el puerto al formato de red, porque aparentemente hasta los números necesitan pasaporte cuando viajan por sockets.

### 3. Conectar con el servidor

La función `ConectarServidor()` usa `connect()` para conectar el socket del cliente con el servidor:

```cpp
connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
```

Devuelve `true` si conecta correctamente y `false` si ocurre algún error.

### 4. Recibir datos

La función `RecibirDatos()` recibe los datos del socket:

```cpp
std::vector<float> RecibirDatos(int sockfd){
    std::vector<float> floats;
    float buffer;

    while (recv(sockfd, &buffer, sizeof(buffer), 0) > 0) {
        floats.push_back(buffer);
    }
    return floats;
}
```

Cada llamada a `recv()` intenta leer `sizeof(float)` bytes y los guarda directamente dentro de una variable `float`.

Esto significa que los bytes recibidos no se convierten de forma matemática, sino que se interpretan como si ya fueran la representación binaria de un `float`.

### 5. Guardar datos

La función `Guardar()` abre `datos.txt` y escribe cada valor en una línea:

```cpp
for (float f : floats) {
    file << f << std::endl;
}
```

### 6. Imprimir datos

La función `imprimir()` muestra los valores recibidos por consola:

```cpp
for (float f : floats) {
    std::cout << f << std::endl;
}
```

---

## Compilación

Para compilar el cliente C++:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic cliente.cpp libreria.cpp -o cliente
```

Esto genera el ejecutable:

```text
cliente
```

---

## Ejecución

Antes de ejecutar el script, dale permisos:

```bash
chmod +x ejecutar.sh
```

### Ejecución normal con servidor simulado

```bash
./ejecutar.sh
```

Este modo:

1. Inicia `ServidorCamara.py` en segundo plano.
2. Ejecuta `./cliente`.
3. Espera a que el cliente termine.
4. Cierra el servidor simulado.

### Ejecución sin servidor simulado

```bash
./ejecutar.sh -s
```

Este modo no inicia `ServidorCamara.py`. Solo ejecuta el cliente.

Sirve si ya tienes otro servidor escuchando en la IP y puerto configurados. Si no hay servidor, el cliente fallará al conectar. Sorpresa: para conectarse a un servidor hace falta un servidor.

---

## Gestión del servidor en el script

El script `ejecutar.sh` guarda el PID del servidor Python con:

```sh
SERVER_PID=$!
```

Después de que el cliente termina, intenta cerrar el servidor con una cadena de señales:

1. `SIGTERM`: cierre normal.
2. `SIGINT`: interrupción similar a `Ctrl + C`.
3. `SIGKILL`: cierre forzado desde el kernel.

Fragmento principal:

```sh
kill -TERM "$SERVER_PID"
sleep 4

if kill -0 "$SERVER_PID" 2>/dev/null; then
    kill -INT "$SERVER_PID"
    sleep 4

    if kill -0 "$SERVER_PID" 2>/dev/null; then
        kill -KILL "$SERVER_PID"
    fi
fi
```

`kill -0` no mata el proceso. Solo comprueba si sigue existiendo.

---

## Archivo de salida

El programa genera el archivo:

```text
datos.txt
```

Ejemplo de salida:

```text
3.82047e-37
1.00825e-34
2.65846e-32
7.00365e-30
1.84362e-27
...
```

Estos valores aparecen porque el cliente interpreta grupos de bytes como si fueran números `float`.

---

## Nota importante sobre los datos recibidos

El servidor Python envía **768 bytes**:

```python
struct.pack('768B', *data_array)
```

El cliente C++ recibe esos bytes en bloques del tamaño de un `float`:

```cpp
recv(sockfd, &buffer, sizeof(buffer), 0)
```

Como normalmente un `float` ocupa 4 bytes, 768 bytes pueden producir hasta:

```text
768 / 4 = 192 floats
```

Pero hay un detalle técnico importante: TCP es un flujo de bytes, no un sistema de paquetes perfectamente alineados para tu comodidad emocional. Una llamada a `recv()` no garantiza siempre recibir exactamente 4 bytes. Para una práctica local puede funcionar, pero en un programa más robusto habría que comprobar cuántos bytes devuelve `recv()` y reconstruir cada `float` correctamente.

---

## Control básico de errores

El programa incluye controles simples:

- Si `socket()` falla, `CrearSocket()` devuelve `-1`.
- Si la IP no es válida, `CrearDireccionServidor()` marca la dirección como no utilizable.
- Si `connect()` falla, `ConectarServidor()` devuelve `false`.
- Si no se puede abrir `datos.txt`, `Guardar()` devuelve `false`.
- El script intenta cerrar el servidor con `SIGTERM`, luego `SIGINT` y finalmente `SIGKILL`.

---

## Requisitos

- Linux o sistema compatible con sockets POSIX.
- `g++` con soporte para C++17.
- Python 3.
- Shell compatible con `/bin/sh`.

En sistemas Debian/Kali/Ubuntu puedes instalar lo necesario con:

```bash
sudo apt update
sudo apt install g++ python3
```

---

## Resumen técnico

Este proyecto demuestra una comunicación cliente-servidor básica mediante sockets TCP/IP.

El servidor Python envía datos binarios, el cliente C++ los recibe usando `recv()`, los almacena en un `std::vector<float>`, los escribe en `datos.txt` y los imprime por pantalla.

La división en `cliente.cpp`, `libreria.cpp`, `libreria.h` y `config.h` facilita mantener el código, modificar partes concretas y evitar que todo acabe en un único archivo monstruoso, que es la forma tradicional humana de invocar el sufrimiento.
