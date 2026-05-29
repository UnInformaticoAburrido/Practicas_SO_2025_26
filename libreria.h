#ifndef LIBRERIA_H
#define LIBRERIA_H

#include <string>
#include <vector>

const std::string rutaArchivoJson = "tactile_captures_50.json";
const std::string servidor = "127.0.0.1";
const int puerto = 5000;
const std::string ruta = "/captura";
const int factorEscala = 8;

std::vector<std::vector<std::vector<int>>> leer_json(const std::string& rutaArchivo, int& codigoError);
int validar_capturas(const std::vector<std::vector<std::vector<int>>>& capturas);
std::vector<std::vector<double>> interpolar_bilineal(const std::vector<std::vector<int>>& matriz16, int factor);
int enviar_http(const std::string& servidor, int puerto, const std::string& rutaHttp, int id, const std::vector<std::vector<double>>& datos);

#endif
