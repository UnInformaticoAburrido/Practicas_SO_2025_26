#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <cstdio> // necesario para usar remove() y eliminar archivos temporales

using namespace std;

int main() {
    int contador_total_registros = 0; // Contador total de registros procesados
    // apertura del archivo de datos
    ifstream archivo_entrada("datos_pinza.txt");
    // Creacion del archivo de salida.
    ofstream archivo_salida("resultado_pinza.txt");
    float suma_galga_total = 0, suma_izq_total = 0, suma_der_total = 0;
    float max_fuerza = -1e9, min_fuerza = 1e9;
    if (!archivo_entrada.is_open()) {
        cout << "Error: no se pudo abrir el archivo" << endl;
        return 1;
    }else if (!archivo_salida.is_open())
    {
        cout << "Error: no se pudo abrir el archivo de salida" << endl;
        return 1;
    }else{
        //escritura del encabezado
        archivo_salida << "RESULTADOS DEL SISTEMA DE PINZA ROBOTICA" << endl;
        ofstream temp_datos("temp_datos");
        ofstream temp_clasificacion("temp_clasificacion");
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
                temp_datos << "ID:" << ids[registrosLeidos] << " Galga:" << galga[registrosLeidos]
                           << " Izq:" << fuerza_izq[registrosLeidos]
                           << " Der:" << fuerza_der[registrosLeidos] << endl;
                registrosLeidos++;
            }
            if (registrosLeidos < 100) {
                if (!archivo_entrada.eof())
                {
                    cout << "Error: se produjo un error al leer el archivo" << endl;
                    return 1;
                }
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
                temp_clasificacion << ids[i] << " " << estado[i] << endl;
                float fuerza_actual = fuerza_izq[i] > fuerza_der[i] ? fuerza_izq[i] : fuerza_der[i];
                if (fuerza_actual > max_fuerza) {
                    max_fuerza = fuerza_actual;
                }
                if (fuerza_actual < min_fuerza) {
                    min_fuerza = fuerza_actual;
                }
            }
            if (registrosLeidos != 100) {
                //Comnprobamos si se han leido registros alamenos una vez.
                if (contador_total_registros == 0) {
                    if (archivo_entrada.eof()) {
                        cout << "Error: el archivo esta vacio o no contiene datos validos" << endl;
                        return 1;
                    }
                }
                //Sumaos el ultimo bloque
                contador_total_registros += registrosLeidos; // Actualizamos el contador total de registros procesados
                suma_galga_total += suma_galga;
                suma_izq_total += suma_izq;
                suma_der_total += suma_der;
                temp_datos.close();
                temp_clasificacion.close();
                // Calculo y registro de los promedios finales
                archivo_salida << "\nPROMEDIOS:" << endl;
                archivo_salida << "Galga: " << (suma_galga_total / contador_total_registros) << endl;
                archivo_salida << "Fuerza Izq: " << (suma_izq_total / contador_total_registros) << endl;
                archivo_salida << "Fuerza Der: " << (suma_der_total / contador_total_registros) << endl;
                archivo_salida << "Valor maximo de fuerza: " << max_fuerza << endl;
                archivo_salida << "Valor minimo de fuerza: " << min_fuerza << endl;
                archivo_salida << "\nDATOS:" << endl;
                ifstream lectura_datos("temp_datos");
                string linea;
                while (getline(lectura_datos, linea)) {
                    archivo_salida << linea << endl;
                }
                //imprimismos los promedios y max/min en consola
                cout << "\nPROMEDIOS:" << endl;
                cout << "Galga: " << (suma_galga_total / contador_total_registros) << endl;
                cout << "Fuerza Izq: " << (suma_izq_total / contador_total_registros) << endl;
                cout << "Fuerza Der: " << (suma_der_total / contador_total_registros) << endl;
                cout << "Valor maximo de fuerza: " << max_fuerza << endl;
                cout << "Valor minimo de fuerza: " << min_fuerza << endl;
                archivo_salida << "\nCLASIFICACION:" << endl;
                ifstream lectura_clasificacion("temp_clasificacion");
                string linea_clas;
                while (getline(lectura_clasificacion, linea_clas)) {
                    archivo_salida << linea_clas << endl;
                }
                //Cerramos los archivos y eliminamos los temporales
                archivo_entrada.close();
                archivo_salida.close();
                //eliminamos archivos temporales
                remove("temp_datos");
                remove("temp_clasificacion");
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