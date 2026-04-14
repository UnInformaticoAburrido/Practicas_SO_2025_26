# Práctica 2 – Procesamiento de datos de sensores en una pinza robótica (C++)

## Descripción del proyecto

Este proyecto implementa un programa en C++ que procesa datos experimentales obtenidos de los sensores de una pinza robótica. El programa lee los datos desde un archivo de entrada, los almacena en arrays y matrices, calcula medias de los sensores, analiza la estabilidad del agarre y genera un archivo de resultados.

Además, se incluye un script en Bash que automatiza la ejecución del programa en segundo plano y gestiona su finalización.

---

## Estructura del proyecto

El repositorio contiene los siguientes archivos:

* `progrmaC.cpp` → Código fuente del programa en C++
* `datos_pinza.txt` → Archivo de entrada con los datos de sensores
* `resultado_pinza.txt` → Archivo generado con los resultados del análisis
* `script.sh` → Script Bash para ejecutar el programa automáticamente
* `README.md` → Documento de instrucciones del proyecto
* `informe.pdf` → Informe explicativo de la práctica

---

## Funcionamiento del programa

El programa realiza las siguientes tareas:

1. Lee los datos desde el archivo `datos_pinza.txt`
2. Almacena los datos en matrices y arrays independientes
3. Muestra los datos por pantalla
4. Calcula la media de:

   * Galga extensométrica
   * Fuerza izquierda
   * Fuerza derecha
5. Evalúa la estabilidad del agarre comparando las fuerzas izquierda y derecha
6. Genera el archivo `resultado_pinza.txt` con los resultados

Una muestra se considera **INESTABLE** si:

abs(fuerza_izq - fuerza_der) > 0.15

En caso contrario se considera **ESTABLE**.

---

## Compilación del programa

Para compilar el programa:

```bash
g++ progrmaC.cpp -o progrmaC
```

---

## Ejecución del programa

Ejecutar directamente:

```bash
./progrmaC
```

---

## Ejecución mediante script Bash

1. Dar permisos de ejecución:

```bash
chmod +x ejecutar_pinza.sh
```

2. Ejecutar el script:

```bash
./ejecutar_pinza.sh
```

El script realiza:

* ejecución del binario mediante `sh` (en segundo plano)
* espera de finalización del proceso
* comprobación del estado del proceso
* finalización controlada si sigue activo

---

## Archivo de entrada

Formato del archivo `datos_pinza.txt`:

ID GALGA FUERZA_IZQ FUERZA_DER

Ejemplo:

5 0.245 1.82 1.76
2 0.198 1.65 1.60
8 0.301 1.95 1.91
1 0.187 1.58 1.55

---

## Archivo de salida

El programa genera:

`resultado_pinza.txt`

Incluye:

* datos procesados
* medias de los sensores
* clasificación de estabilidad de cada muestra

---

## Requisitos del sistema

Sistema operativo Linux
Compilador compatible con C++ (g++ recomendado)
Bash disponible en el sistema o wsl isntalado en el sistema
