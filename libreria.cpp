#include "config.h"
#include "libreria.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

int CrearSocket(){
    int test = socket(AF_INET, SOCK_STREAM, 0);

    if (test == -1) {
        std::cerr << "Error al crear el socket" << std::endl;
        return -1;
    }
    return test;
}
sockaddr_in CrearDireccionServidor() {
    sockaddr_in servaddr_out{};
    servaddr_out.sin_family = AF_INET;
    servaddr_out.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &servaddr_out.sin_addr) <= 0) {
        std::cerr << "Direccion invalida o no soportada" << std::endl;
        servaddr_out.sin_family = AF_UNSPEC;
    }

    return servaddr_out;
}
bool ConectarServidor(int sockfd, sockaddr_in servaddr) {
    
    if (servaddr.sin_family == AF_UNSPEC) {
        return false;
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        std::cerr << "Error al conectar" << std::endl;
        return false;
    }

    return true;
}
std::vector<float> RecibirDatos(int sockfd){
    std::vector<float> floats;
    float buffer;

    while (recv(sockfd, &buffer, sizeof(buffer), 0) > 0) {
        floats.push_back(buffer);
    }
    return floats;
}
bool Guardar(std::vector<float> floats){
    std::ofstream file(FILE_PATH);
    if (file.is_open()) {
        for (float f : floats) {
            file << f << std::endl;
        }
        file.close();
        return true;
    }
    return false;
}
void imprimir(std::vector<float> floats){
    std::cout << "Numeros de punto flotante recibidos:" << std::endl;
    for (float f : floats) {
        std::cout << f << std::endl;
    }
}