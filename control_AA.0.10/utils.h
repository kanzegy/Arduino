// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// Funci�n para convertir un n�mero de punto flotante a string
char* _2_string(double numero) {
    static char resultado[20]; // Buffer para almacenar el string
    dtostrf(numero, 2, 1, resultado); // Convierte el double en string
    return resultado; // Retorna el puntero al string
}

// Funci�n para convertir un n�mero entero a string
char* _2_string(int numero) {
    static char resultado[12]; // Buffer para almacenar el string (suficiente para int)
    itoa(numero, resultado, 10); // Convierte el entero en string en base 10
    return resultado; // Retorna el puntero al string
}

// Funci�n para convertir un n�mero entero a string con dos d�gitos
char* _2_digit_number(int valor){
    static char data[3]; // Buffer para almacenar el string de dos d�gitos
    if(valor > 9){
        strcpy(data, _2_string(valor));
    }
    else{
        data[0] = '0'; // Agrega el cero en la primera posici�n
        data[1] = _2_string(valor)[0]; // Copia el d�gito en la segunda posici�n
        data[2] = '\0'; // Termina la cadena
    }
    return data;
}

#endif // UTILS_H
