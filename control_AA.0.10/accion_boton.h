// accion_boton.h
#ifndef ACCION_BOTON_H
#define ACCION_BOTON_H

#include <Arduino.h>

struct accion_boton {
    int x = -1;
    int y = -1;
    int largo = -1;
    int arr = -1;
    int aba = -1;
    int atr = -1;
    int ade = -1;
    int max = -1;
    int min = -1;
    int* parametro_afectado;
    char alias;
    char alias_str[2]; // Ajuste para incluir el car�cter y el terminador null

    // Constructor
    accion_boton(int _x = -1, int _y = -1, int _largo = -1, int _arriba = -1, 
                int _abajo = -1, int _atras = -1, int _adelante = -1, 
                int _max = -1, int _min = -1, int* param = nullptr, char _alias = 'D'){
                  
        x = _x;
        y = _y;
        largo = _largo;
        arr = _arriba;
        aba = _abajo;
        atr = _atras;
        ade = _adelante;
        max = _max;
        min = _min;
        parametro_afectado = param;
        alias = _alias;
        alias_str[0] = alias;
        alias_str[1] = '\0'; // Terminar la cadena
    } 

    // Funci�n para modificar los valores de max y min
    void setMaxMin(int nuevo_max, int nuevo_min) {
        max = nuevo_max;
        min = nuevo_min;
    }

    // Funci�n preciona_arriba: Incrementa el valor de parametro_afectado
    int preciona_arriba() {
        Serial.println("OK");
        if (parametro_afectado != nullptr && *parametro_afectado < max) {
            (*parametro_afectado)++;
        }
        return arr;
    }

    // Funci�n preciona_abajo: Reduce el valor de parametro_afectado
    int preciona_abajo() {
        Serial.println("OK");
        if (parametro_afectado != nullptr && *parametro_afectado > min) {
            (*parametro_afectado)--;
        }
        return aba;
    }

    // Funci�n preciona_atras: Retorna el valor de atras
    int preciona_atras() {
        Serial.println("OK");
        return atr;
    }

    // Funci�n preciona_adelante: Retorna el valor de adelante
    int preciona_adelante() {
        Serial.println("OK");
        return ade;
    }
};

#endif // ACCION_BOTON_H
