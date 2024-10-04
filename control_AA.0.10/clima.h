// clima.h
#ifndef CLIMA_H
#define CLIMA_H

#include <Arduino.h>
#include <math.h>
#include <TimeLib.h>

enum Estados {
    h_apagado = 0,
    h_encendido = 1,
    d_mostrar_temperaturas = 2,
    rare = 999
};

struct Clima {
    int sensor;
    int pin_compresor;
    bool es_principal;
    Estados estado;
    time_t principal_desde;
    time_t hora_encendido;
    int temp;
    bool alarma;
    Clima(int _sensor, int _pin_compresor, bool _es_principal, Estados _estado, time_t _principal_desde) {
      sensor = _sensor;
      pin_compresor = _pin_compresor;
      es_principal = _es_principal;
      estado = _estado;
      principal_desde = _principal_desde;

      pinMode(pin_compresor, OUTPUT);
      digitalWrite(pin_compresor, LOW);
    }

    void apagar() {
        if(estado == h_encendido){
            estado = h_apagado;
            digitalWrite(pin_compresor, LOW);
        }
    }

    void encender() {
        if(estado == h_apagado){
            estado = h_encendido;
            hora_encendido = now();
            digitalWrite(pin_compresor, HIGH);
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

    bool hay_alarma(){
      //logica para leer y validar pin
      alarma = false;
      return alarma;
    }
};

#endif // CLIMA_H
