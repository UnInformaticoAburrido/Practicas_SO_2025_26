#include "libreria.h"

#include <algorithm>
#include <cmath>
#include <curl/curl.h>
#include <fstream>
#include "nlohmann/json.hpp"
#include <sstream>
#include <string>

using json = nlohmann::json;

std::vector<std::vector<std::vector<int>>> leer_json(const std::string& rutaArchivo, int& codigoError) {
    codigoError = 0;

    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        codigoError = 1;
        return {};
    }

    try {
        json datos;
        archivo >> datos;

        if (!datos.contains("captures") || !datos["captures"].is_array()) {
            codigoError = 1;
            return {};
        }

        std::vector<std::vector<std::vector<int>>> capturas;

        for (const auto& captura : datos["captures"]) {
            if (!captura.contains("matrix") || !captura["matrix"].is_array()) {
                codigoError = 1;
                return {};
            }

            capturas.push_back(captura["matrix"].get<std::vector<std::vector<int>>>());
        }

        if (capturas.empty()) {
            codigoError = 1;
        }

        return capturas;
    } catch (...) {
        codigoError = 1;
        return {};
    }
}

int validar_capturas(const std::vector<std::vector<std::vector<int>>>& capturas) {
    if (capturas.empty()) {
        return 1;
    }

    for (std::size_t i = 0; i < capturas.size(); i++) {
        if (capturas[i].size() != 16) {
            return 1;
        }

        for (std::size_t fila = 0; fila < capturas[i].size(); fila++) {
            if (capturas[i][fila].size() != 16) {
                return 1;
            }
        }
    }

    return 0;
}

std::vector<std::vector<double>> interpolar_bilineal(const std::vector<std::vector<int>>& matriz16, int factor) {
    int altoOriginal = static_cast<int>(matriz16.size());
    int anchoOriginal = static_cast<int>(matriz16[0].size());
    int altoNuevo = altoOriginal * factor;
    int anchoNuevo = anchoOriginal * factor;

    std::vector<std::vector<double>> matrizInterpolada(altoNuevo, std::vector<double>(anchoNuevo, 0.0));

    for (int y = 0; y < altoNuevo; y++) {
        double yOriginal = 0.0;
        if (altoNuevo > 1) {
            yOriginal = static_cast<double>(y) * (altoOriginal - 1) / (altoNuevo - 1);
        }

        int y0 = static_cast<int>(std::floor(yOriginal));
        int y1 = std::min(y0 + 1, altoOriginal - 1);
        double dy = yOriginal - y0;

        for (int x = 0; x < anchoNuevo; x++) {
            double xOriginal = 0.0;
            if (anchoNuevo > 1) {
                xOriginal = static_cast<double>(x) * (anchoOriginal - 1) / (anchoNuevo - 1);
            }

            int x0 = static_cast<int>(std::floor(xOriginal));
            int x1 = std::min(x0 + 1, anchoOriginal - 1);
            double dx = xOriginal - x0;

            double valor00 = matriz16[y0][x0];
            double valor10 = matriz16[y0][x1];
            double valor01 = matriz16[y1][x0];
            double valor11 = matriz16[y1][x1];

            double valorSuperior = valor00 * (1.0 - dx) + valor10 * dx;
            double valorInferior = valor01 * (1.0 - dx) + valor11 * dx;

            matrizInterpolada[y][x] = valorSuperior * (1.0 - dy) + valorInferior * dy;
        }
    }

    return matrizInterpolada;
}

std::string convertir_matriz_a_json(int id, const std::vector<std::vector<double>>& datos) {
    json cuerpo;
    cuerpo["capture_id"] = id;
    cuerpo["width"] = datos.empty() ? 0 : datos[0].size();
    cuerpo["height"] = datos.size();
    cuerpo["data"] = datos;

    return cuerpo.dump();
}

std::size_t descartar_respuesta(char* contenido, std::size_t tamano, std::size_t cantidad, void* usuario) {
    return tamano * cantidad;
}

int enviar_http(const std::string& servidor, int puerto, const std::string& rutaHttp, int id, const std::vector<std::vector<double>>& datos) {
    if (servidor.empty() || puerto <= 0 || rutaHttp.empty() || rutaHttp[0] != '/') {
        return 1;
    }

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        return 1;
    }

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        curl_global_cleanup();
        return 1;
    }

    std::string url = "http://" + servidor + ":" + std::to_string(puerto) + rutaHttp;
    std::string cuerpo = convertir_matriz_a_json(id, datos);

    curl_slist* cabeceras = nullptr;
    cabeceras = curl_slist_append(cabeceras, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, cabeceras);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cuerpo.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(cuerpo.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, descartar_respuesta);

    CURLcode resultado = curl_easy_perform(curl);
    if (resultado != CURLE_OK) {
        curl_slist_free_all(cabeceras);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    long codigoHttp = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigoHttp);

    curl_slist_free_all(cabeceras);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (codigoHttp != 200 && codigoHttp != 201) {
        return 1;
    }

    return 0;
}
