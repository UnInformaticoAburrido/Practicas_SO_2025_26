#include "libreria.h"

#include <iostream>
#include <vector>

int main() {
    int errorLectura = 0;
    std::vector<std::vector<std::vector<int>>> capturas = leer_json(rutaArchivoJson, errorLectura);

    if (errorLectura != 0) {
        std::cout << "Error: no se pudo leer o parsear el archivo JSON '" << rutaArchivoJson << "'." << std::endl;
        std::cout << "Comprueba que el archivo existe y que contiene el campo captures con matrices validas." << std::endl;
        return 1;
    }

    int errorValidacion = validar_capturas(capturas);
    if (errorValidacion != 0) {
        std::cout << "Error: las capturas no tienen el formato esperado." << std::endl;
        std::cout << "Todas las matrices deben tener exactamente 16 filas y 16 columnas." << std::endl;
        return 1;
    }

    std::cout << "Capturas cargadas correctamente: " << capturas.size() << std::endl;

    for (std::size_t i = 0; i < capturas.size(); i++) {
        std::vector<std::vector<double>> matrizInterpolada = interpolar_bilineal(capturas[i], factorEscala);

        int errorEnvio = enviar_http(servidor, puerto, ruta, static_cast<int>(i), matrizInterpolada);
        if (errorEnvio != 0) {
            std::cout << "Error: no se pudo enviar la captura " << i << " al servidor." << std::endl;
            std::cout << "Comprueba que servidor.py esta ejecutandose en " << servidor << ":" << puerto << ruta << "." << std::endl;
            return 1;
        }

        std::cout << "Captura " << i << " enviada correctamente." << std::endl;
    }

    std::cout << "Proceso completado correctamente." << std::endl;
    return 0;
}
