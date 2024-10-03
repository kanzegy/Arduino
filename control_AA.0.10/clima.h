// clima.h
#ifndef CLIMA_H
#define CLIMA_H

#include <Arduino.h>
#include <math.h>
#include <TimeLib.h>

enum Estados {
    h_apagado = 1,
    h_encendido = 2,
    d_mostrar_temperaturas = 0,
    rare = 999
};

struct Clima {
    int sensor;
    bool es_principal;
    Estados estado;
    time_t principal_desde;
    time_t hora_encendido;
    int temp;

    void apagar() {
        if(estado == h_encendido){
            estado = h_apagado;
        }
    }

    void encender() {
        if(estado == h_apagado){
            estado = h_encendido;
            hora_encendido = now();
        }
    }

    int leer_temperatura() {
        int val = analogRead(sensor);
        double temp_temp = log(((10240000.0 / val) - 10000.0));
        temp_temp = 1.0 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp_temp * temp_temp)) * temp_temp);
        temp_temp = temp_temp - 273.15;
        temp = (int)temp_temp;
        return temp;
    }

    long tiempo_encendido() {
        if (estado == h_apagado) return -1;
        return now() - hora_encendido; // Diferencia en segundos
    }

    long tiempo_siendo_principal() {
        if (!es_principal) return -1;
        return (now() - principal_desde) / 86400; // 86400 segundos en un d�a
    }
};

#endif // CLIMA_H
