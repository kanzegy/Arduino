// estado_hw.h
#ifndef ESTADO_HW_H
#define ESTADO_HW_H

#include "clima.h"

struct estado_hw {
    Clima* principal;
    Clima* secundario;
    int lugar_actual; 
    int lugar_anterior; 
    int temp_max;
    int umbral;
    int ploteo_dias;

    void verifica_ploteo() {
        if (principal->tiempo_siendo_principal() >= ploteo_dias){
            // Intercambio entre principal y secundario
            Clima* temp = principal;
            principal = secundario;
            secundario = temp;

            // Cambiar el valor de es_principal
            principal->es_principal = true;
            principal->principal_desde = now();
            
            secundario->es_principal = false;
        }
    }

    void verifica_temperaturas(){
        principal->leer_temperatura(); // Actualizamos el valor de la temperatura
        secundario->leer_temperatura();
        bool max_revasado = (principal->temp > temp_max || secundario->temp > temp_max);
        bool min_alcanzado = (principal->temp < (temp_max - umbral) || secundario->temp < (temp_max - umbral));
        if(min_alcanzado){
            principal->apagar();
            secundario->apagar();
        }
        else if(max_revasado){
            if(principal->estado == h_apagado && (secundario->estado == h_apagado || secundario->tiempo_encendido() > 5*60/*5 MINUTOS*/)){
                principal->encender();
            }
            else if(principal->tiempo_encendido() > 5*60/*5 MINUTOS*/){
                secundario->encender();
            }
        }
    }
};

#endif // ESTADO_HW_H
