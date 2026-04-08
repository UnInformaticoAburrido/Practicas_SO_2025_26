#include <iostream>
#include <fstream>
#include <cmath>
#include <string>

using namespace std;

int main() {
    // Definicion de variables y matrices 
    float datos[100][3]; 
    int ids[100];
    float galga[100], fuerza_izq[100], fuerza_der[100];
    string estado[100];

    // apertura del archivo de datos
    ifstream archivo_entrada("datos_pinza.txt");
    if (!archivo_entrada.is_open()) {
        cout << "Error: no se pudo abrir el archivo" << endl;
        return 1;
    }

    cout << "Iniciando el procesamiento de datos" << endl;

    int n = 0;
    // Lectura de los primeros 100 registros
    while (n < 100 && archivo_entrada >> ids[n] >> datos[n][0] >> datos[n][1] >> datos[n][2]) {
        galga[n] = datos[n][0];
        fuerza_izq[n] = datos[n][1];
        fuerza_der[n] = datos[n][2];
        n++;
    }
    archivo_entrada.close();
    // Verificamos que se hayan leído algún dato.
    if (n == 0) {
    cout << "Error: no se leyeron datos validos" << endl;
    return 1;
    }

    // Procesamiento y calculo de estabilidad
    float suma_galga = 0, suma_izq = 0, suma_der = 0;
    for (int i = 0; i < n; i++) {
        suma_galga += galga[i];
        suma_izq += fuerza_izq[i];
        suma_der += fuerza_der[i];

        // Calculo de la diferencia absoluta para la estabilidad
        float diferencia = fuerza_izq[i] - fuerza_der[i];
        if (abs(diferencia) > 0.15) {
            estado[i] = "INESTABLE";
        } else {
            estado[i] = "ESTABLE";
        }
    }
    
    // Escritura de todos los resultados en el archivo de salida
    ofstream archivo_salida("resultado_pinza.txt");
    archivo_salida << "ID      GALGA    IZQ    DER     ESTADO" << endl;

    for (int i = 0; i < n; i++) {
        archivo_salida << ids[i] << "\t" << galga[i] << "\t" << fuerza_izq[i] 
                       << "\t" << fuerza_der[i] << "\t" << estado[i] << endl;
    }

    // Calculo y registro de los promedios finales
    archivo_salida << "\nPROMEDIOS:" << endl;
    archivo_salida << "Galga: " << (suma_galga / n) << endl;
    archivo_salida << "Fuerza Izq: " << (suma_izq / n) << endl;
    archivo_salida << "Fuerza Der: " << (suma_der / n) << endl;

    archivo_salida.close();

    
    cout << "Resultados guardados en 'resultado_pinza.txt'." << endl;

    return 0;
}