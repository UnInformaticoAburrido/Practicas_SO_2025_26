#include <iostream>
#include <fstream>
#include <cmath>
#include <string>

using namespace std;

int main() {
    int contador_total_registros = 0; // Contador total de registros procesados
    // apertura del archivo de datos
    ifstream archivo_entrada("datos_pinza.txt");
    // Creacion del archivo de salida.
    ofstream archivo_salida("resultado_pinza.txt");
    float suma_galga_total = 0, suma_izq_total = 0, suma_der_total = 0;
    if (!archivo_entrada.is_open()) {
        cout << "Error: no se pudo abrir el archivo" << endl;
        return 1;
    }else if (!archivo_salida.is_open())
    {
        cout << "Error: no se pudo abrir el archivo de salida" << endl;
        return 1;
    }else{
        //escritura del encabezado
        archivo_salida << "ID      GALGA    IZQ    DER     ESTADO" << endl;
        while (true)
        {
            // Definicion de variables y matrices 
            float datos[100][3]; 
            int ids[100];
            float galga[100], fuerza_izq[100], fuerza_der[100];
            string estado[100];
            int registrosLeidos = 0; // Contador de registros leidos durante el bucle
            // Lectura de los primeros 100 registros
            while (registrosLeidos < 100 && archivo_entrada >> ids[registrosLeidos] >> datos[registrosLeidos][0] >> datos[registrosLeidos][1] >> datos[registrosLeidos][2]) {
                galga[registrosLeidos] = datos[registrosLeidos][0];
                fuerza_izq[registrosLeidos] = datos[registrosLeidos][1];
                fuerza_der[registrosLeidos] = datos[registrosLeidos][2];
                registrosLeidos++;
            }
            if (registrosLeidos < 100 && archivo_entrada.eof()) {
                cout << "Alerta: no se leyeron datos validos de L:" << registrosLeidos << "en el bloque " << contador_total_registros/100 << endl;
            }
            // Procesamiento y calculo de estabilidad
            float suma_galga = 0, suma_izq = 0, suma_der = 0;
            for (int i = 0; i < registrosLeidos; i++) {
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
            for (int i = 0; i < registrosLeidos; i++) {
                archivo_salida << ids[i] << "\t" << galga[i] << "\t" << fuerza_izq[i] << "\t" << fuerza_der[i] << "\t" << estado[i] << endl;
            }
            if (registrosLeidos != 100) {
                //Comnprobamos si se han leido registros alamenos una vez.
                
                if (contador_total_registros == 0) {
                    if (archivo_entrada.eof()) {
                        cout << "Alerta: el archivo esta vacio o no contiene datos validos" << endl;
                        return 1;
                    }
                }
                //Sumaos el ultimo bloque
                contador_total_registros += registrosLeidos; // Actualizamos el contador total de registros procesados
                suma_galga_total += suma_galga;
                suma_izq_total += suma_izq;
                suma_der_total += suma_der;
                // Calculo y registro de los promedios finales
                archivo_salida << "\nPROMEDIOS:" << endl;
                archivo_salida << "Galga: " << (suma_galga_total / contador_total_registros) << endl;
                archivo_salida << "Fuerza Izq: " << (suma_izq_total / contador_total_registros) << endl;
                archivo_salida << "Fuerza Der: " << (suma_der_total / contador_total_registros) << endl;
                //imprimismos los promedios en consola
                cout << "\nPROMEDIOS:" << endl;
                cout << "Galga: " << (suma_galga_total / contador_total_registros) << endl;
                cout << "Fuerza Izq: " << (suma_izq_total / contador_total_registros) << endl;
                cout << "Fuerza Der: " << (suma_der_total / contador_total_registros) << endl;
                //Cerramos los archivos
                archivo_entrada.close();
                archivo_salida.close();
                cout << "Resultados guardados en 'resultado_pinza.txt'." << endl;
                return 0;
            }else{
                contador_total_registros += registrosLeidos; // Actualizamos el contador total de registros procesados
                suma_galga_total += suma_galga;
                suma_izq_total += suma_izq;
                suma_der_total += suma_der;
            }
        }
    }
}