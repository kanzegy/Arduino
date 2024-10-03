#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>  // Incluir la librería EEPROM

struct Config {
  int temp_max;
  int umbral;
  int f_anio;
  int f_mes;
  int f_dia;
  int f_hora;
  int f_min;
  int ploteo_dias;

  // Función para guardar la configuración en la EEPROM
  void guarda_eprom() {
    EEPROM.put(0, *this);  // Guardar toda la estructura en la EEPROM desde la posición 0
  }

  // Función para cargar la configuración desde la EEPROM
  void carga_eprom() {
    EEPROM.get(0, *this);  // Cargar toda la estructura desde la EEPROM desde la posición 0
  }

  // Función para abrir la configuración con parámetros
  void abrir_config(int _temp_max, int _umbral, int _f_anio, int _f_mes, int _f_dia, int _f_hora, int _f_min, int _ploteo_dias) {
    temp_max = _temp_max;
    umbral = _umbral;
    f_dia = _f_dia;
    f_mes = _f_mes;
    f_anio = _f_anio;
    f_hora = _f_hora;
    f_min = _f_min;
    ploteo_dias = _ploteo_dias;
  }
};

#endif
