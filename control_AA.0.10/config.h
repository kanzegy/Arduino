#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>  // Incluir la librería EEPROM

#define EEPROM_FLAG_ADDR 0      // Dirección en EEPROM para la bandera
#define EEPROM_CONFIG_ADDR sizeof(int)  // Dirección en EEPROM para los datos de configuración
#define EEPROM_FLAG_VALUE 0xA5A5  // Valor que actúa como la bandera

struct Config {
  int temp_max;
  int umbral;
  int dia_ploteo;
  int f_anio;
  int f_mes;
  int f_dia;
  int f_hora;
  int f_min;
  int f_seg;

  // Función para guardar la configuración en la EEPROM
  void guarda_eprom() {
    int flag = EEPROM_FLAG_VALUE;  // Establecer la bandera para indicar que los datos son válidos
    EEPROM.put(EEPROM_FLAG_ADDR, flag);  // Guardar la bandera en la dirección 0
    EEPROM.put(EEPROM_CONFIG_ADDR, *this);  // Guardar toda la estructura en la EEPROM a partir de la dirección siguiente
  }

  // Función para cargar la configuración desde la EEPROM
  void carga_eprom() {
    int flag;
    EEPROM.get(EEPROM_FLAG_ADDR, flag);  // Leer la bandera desde la EEPROM

    if (flag == EEPROM_FLAG_VALUE) {
      Serial.println("Cargando configuracion guardada.");
      EEPROM.get(EEPROM_CONFIG_ADDR, *this);
    } else {
      Serial.println("EEPROM vacía cargando configuracion default.");
      temp_max = 27;
      umbral = 5;
      dia_ploteo = 1;
      f_anio = 2024;
      f_mes = 10;
      f_dia = 3;
      f_hora = 8;
      f_min = 26;
      f_seg = 0;
    }
  }
  // Función para abrir la configuración con parámetros
  void abrir_config(int _temp_max, int _umbral, int _dia_ploteo, int _f_anio, int _f_mes, int _f_dia, int _f_hora, int _f_min, int _f_seg) {
    temp_max = _temp_max;
    umbral = _umbral;
    dia_ploteo = _dia_ploteo;
    f_dia = _f_dia;
    f_mes = _f_mes;
    f_anio = _f_anio;
    f_hora = _f_hora;
    f_min = _f_min;
    f_seg = _f_seg;
  }
};

#endif
