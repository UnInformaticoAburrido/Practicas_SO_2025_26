#ifndef LIBRERIA_H
#define LIBRERIA_H
#include <iostream>
#include <vector>
#include <cstring>
#include <arpa/inet.h>

int CrearSocket();
sockaddr_in CrearDireccionServidor();
bool Guardar(std::vector<float> floats);
void imprimir(std::vector<float> floats);
bool ConectarServidor(int sockfd, sockaddr_in servaddr);
std::vector<float> RecibirDatos(int sockfd);

#endif