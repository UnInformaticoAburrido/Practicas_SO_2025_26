#include <iostream>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include "libreria.h"
#include "config.h"

int EjecucionConsola();

int main() {
    // Crear un socket TCP/IP
    int sockfd = CrearSocket();
    if (sockfd == -1)
    {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }
    sockaddr_in sockaddr = CrearDireccionServidor();
    // Conectar el socket al servidor
    if (!ConectarServidor(sockfd, sockaddr)){
        std::cerr << "Error al conectar al servidor" << std::endl;
        close(sockfd);
        return 1;
    }
    // Recibir los datos como bytes
    std::vector<float> floats = RecibirDatos(sockfd);
    // Guardar los datos en un archivo de texto
    if (!Guardar(floats)){
        std::cerr << "Error al guardar los datos en el archivo de texto" << std::endl;
        close(sockfd);
        return 1;
    }
    std::cout << "Datos guardados en el archivo de texto" << std::endl;
    // Imprimir los numeros de punto flotante
    imprimir(floats);
    // Cerrar el socket
    close(sockfd);

    return 0;   
}