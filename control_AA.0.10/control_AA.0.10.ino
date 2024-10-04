// main.ino

#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include <TimeLib.h>
#include "virtuabotixRTC.h"
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#include "utils.h"            // Incluir utils.h para las funciones de conversión
#include "accion_boton.h"    // Incluir accion_boton.h
#include "clima.h"           // Incluir clima.h
#include "estado_hw.h"       // Incluir estado_hw.h
#include "config.h"          // Incluir config.h

virtuabotixRTC myRTC(6, 7, 5);
U8G2_ST7565_ERC12864_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=CSL*/ 13, /* data=SI*/ 11, /* cs=CS*/ 10, /* dc=RS*/ 9, /* reset=RSE*/ 8);
time_t momento_actual;

enum comandos_serial {
    arriba = 0,
    abajo = 1,
    atras = 2,
    adelante = 3,
    _440 = 4,
    _490 = 5
};

// Declaración de las instancias de Clima
Clima clima_principal(1, 3, true, h_apagado, now());
Clima clima_secundario(2, 4, false, h_apagado, now());
estado_hw hw = {&clima_principal, &clima_secundario, 0, 0, 27, 5, 7, 0};

// Inicialización de Config
Config hw_config = {0, 0, 0, 0, 0, 0, 0, 0};

// Declaración y inicialización del array de accion_boton
accion_boton lugares[15] = {
              //(x,   y, larg, AR, AB, AT,  AD, MAX, MIN, VARIABLE_A_CONTROLAR, ALIAS), // EJEMPLO
    accion_boton(0,    0,   0, -1, -1, -1,  -1,   0,   0, nullptr              , 'D'), // DASHBOARD - 0
    accion_boton(10,   1,  48,  6,  2,  0,   7,   0,   0, nullptr              , 'T'), // TEMPERATURA - 1
    accion_boton(10,  11,  48,  1,  3,  0,   8,   0,   0, nullptr              , 'U'), // UMBRAL - 2
    accion_boton(10,  21,  48,  2,  4,  0,   9,   0,   0, nullptr              , 'F'), // FECHA - 3
    accion_boton(10,  31,  48,  3,  5,  0,  12,   0,   0, nullptr              , 'H'), // HORA - 4
    accion_boton(10,  41,  48,  4,  6,  0,  14,   0,   0, nullptr              , 'P'), // PLOTEO - 5
    accion_boton(10,  53,  42,  5,  1,  0, 100,   0,   0, nullptr              , 'A'), // ACEPTAR - 6
    accion_boton(68,   1,  41, -1, -1,  1,  -1,  50,  16, &hw_config.temp_max  , 't'), // CONFIG TEMPERATURA - 7
    accion_boton(68,  11,  41, -1, -1,  2,  -1,  20,   1, &hw_config.umbral    , 'u'), // CONFIG UMBRAL - 8
    accion_boton(68,  21,  11, -1, -1,  3,  10,  31,   0, &hw_config.f_dia     , 'd'), // CONFIG DIA - 9
    accion_boton(86,  21,  11, -1, -1,  9,  11,  12,   0, &hw_config.f_mes     , 'm'), // CONFIG MES - 10
    accion_boton(104, 21,  22, -1, -1, 10,  -1,9999,1992, &hw_config.f_anio    , 'a'), // CONFIG AÑO - 11
    accion_boton(68,  31,  12, -1, -1,  4,  13,  23,   0, &hw_config.f_hora    , 'h'), // CONFIG HORA - 12
    accion_boton(86,  31,  12, -1, -1, 12,  -1,  59,   0, &hw_config.f_min     , 'i'), // CONFIG MINUTO - 13
    accion_boton(68,  41,  41, -1, -1,  5,  -1,  28,   1, &hw_config.dia_ploteo, 'p'), // CONFIG PLOTEO - 14
};

// Función para recibir comandos seriales
comandos_serial recibir_comando() {
    if (Serial.available() > 0) {
        String input = Serial.readString(); // Leer el input del puerto serial
        input.trim();
        if (input == "AR") {
            return arriba;
        } else if (input == "AB") {
            return abajo;
        } else if (input == "AT") {
            return atras;
        } else if (input == "AD") {
            return adelante;
        } else if (input == "#440") {
            return _440;
        } else if (input == "#490") {
            return _490;
        }
    }
    return (comandos_serial)(-1); // Retornar -1 si no se recibe un comando válido
}

// Función para ejecutar acciones basadas en el comando recibido
void ejecuta_accion_comando(comandos_serial dir){
    int next = -1;
    switch(dir){
        case arriba: 
            next = lugares[hw.lugar_actual].preciona_arriba(); 
            break;
        case abajo: 
            next = lugares[hw.lugar_actual].preciona_abajo(); 
            break;
        case atras: 
            next = lugares[hw.lugar_actual].preciona_atras(); 
            break;
        case adelante: 
            next = lugares[hw.lugar_actual].preciona_adelante(); 
            break;
        case _440: envia_estatus_440();
            break;
        case _490: hw.lugar_actual = 1; // el 1 es para entrar en la configuracion
            break;
    }
    if(next != -1) {
      hw.lugar_anterior = hw.lugar_actual;
      hw.lugar_actual = next;

      //quiere decir que abrio la configuracion
      if(hw.lugar_anterior == 0 && hw.lugar_actual == 1){
        Serial.println("OK-CONFIG");
        hw_config.abrir_config(hw.temp_max, hw.umbral, hw_config.dia_ploteo, year(momento_actual), month(momento_actual), day(momento_actual), hour(momento_actual), minute(momento_actual), 0);
      }
      //quiere decir que preciono el boton de guardar
      else if(hw.lugar_actual == 100){
        Serial.println("CONFIG-GUARDADA");
        hw_config.guarda_eprom();
        establece_configuracion(true);
        hw.lugar_actual = 0;
      }
    }
}

// Función para actualizar el display
void actualiza_display(){
    if(hw.lugar_actual == 0){
        muestra_temperaturas();
    }
    else{
        muestra_config();
    }
}

// Función para mostrar la configuración en el display
void muestra_config(){
    int ab = 10; // altura base
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_6x10_mr);
        u8g2.drawStr(10, ab, "Temp max");
        u8g2.drawStr(60, ab, ":");
        u8g2.drawStr(68, ab, _2_string(hw_config.temp_max));

        u8g2.drawStr(10, ab + 10, "Umbral");
        u8g2.drawStr(60, ab + 10, ":");
        u8g2.drawStr(68, ab + 10, _2_string(hw_config.umbral));
        
        u8g2.drawStr(10, ab + 20, "Fecha");
        u8g2.drawStr(60, ab + 20, ":");
        u8g2.drawStr(68, ab + 20, "  /  /");
        u8g2.drawStr(68, ab + 20, _2_digit_number(hw_config.f_dia));
        u8g2.drawStr(86, ab + 20, _2_digit_number(hw_config.f_mes));
        u8g2.drawStr(104, ab + 20, _2_string(hw_config.f_anio));

        u8g2.drawStr(10, ab + 30, "Hora");
        u8g2.drawStr(60, ab + 30, ":");
        u8g2.drawStr(68, ab + 30, "  :");
        u8g2.drawStr(68, ab + 30, _2_digit_number(hw_config.f_hora));
        u8g2.drawStr(86, ab + 30, _2_digit_number(hw_config.f_min));
        
        u8g2.drawStr(10, ab + 40, "Dia plot");
        u8g2.drawStr(60, ab + 40, ":");
        u8g2.drawStr(68, ab + 40, _2_string(hw_config.dia_ploteo));
        u8g2.drawStr(85, ab + 40, "d/sem");
        
        u8g2.drawStr(10, ab + 52, "GUARDAR");
        
        u8g2.drawHLine(lugares[hw.lugar_actual].x, ab + lugares[hw.lugar_actual].y, lugares[hw.lugar_actual].largo);

    } while ( u8g2.nextPage() );
}

// Función para mostrar las temperaturas en el display
void muestra_temperaturas(){
    u8g2.firstPage();
    int ab = 8; // altura base
    do {
        u8g2.setFont(u8g2_font_6x10_mr);
        
        u8g2.drawHLine(0, ab + 2, 128);
        u8g2.drawHLine(0, ab + 26, 128);
        u8g2.drawStr(0, ab, "Comp   Tmp   ON   Pri");
        u8g2.drawVLine(32, 0, 34);
        u8g2.drawVLine(68, 0, 34);
        u8g2.drawVLine(98, 0, 34);

        u8g2.drawStr(12, ab + 12, _2_string(hw.principal->sensor));
        u8g2.drawStr(38, ab + 12, _2_string(hw.principal->temp));
        u8g2.drawStr(78, ab + 12, (hw.principal->estado == h_encendido ? "si" : "-"));
        u8g2.drawStr(108, ab + 12, (hw.principal->es_principal ? "si" : "-"));

        u8g2.drawStr(12, ab + 22, _2_string(hw.secundario->sensor));
        u8g2.drawStr(38, ab + 22, _2_string(hw.secundario->temp)); 
        u8g2.drawStr(78, ab + 22, (hw.secundario->estado == h_encendido ? "si" : "-")); 
        u8g2.drawStr(108, ab + 22, (hw.secundario->es_principal ? "si" : "-"));

        u8g2.drawStr(0, ab + 36, "MODE:   | ALARMS:");
        u8g2.drawStr(30, ab + 36, "3");
        u8g2.drawStr(105, ab + 36, "NO");
        
        u8g2.drawStr(0, ab + 55, "  /  /       :  :  ");
        u8g2.drawStr(0, ab + 55, _2_digit_number(day(momento_actual)));
        u8g2.drawStr(18, ab + 55, _2_digit_number(month(momento_actual)));
        u8g2.drawStr(36, ab + 55, _2_string(year(momento_actual)));
        u8g2.drawStr(66, ab + 55, _2_digit_number(hour(momento_actual)));
        u8g2.drawStr(83, ab + 55, _2_digit_number(minute(momento_actual)));
        u8g2.drawStr(102, ab + 55, _2_digit_number(second(momento_actual)));

    } while ( u8g2.nextPage() );
}
void envia_estatus_440(){
  Serial.print(hw.principal->temp);Serial.print("|");
  Serial.print(hw.secundario->temp);Serial.print("|");

  Serial.print(hw.principal->estado);Serial.print("|");
  Serial.print(hw.secundario->estado);Serial.print("|");
  
  Serial.print(hw.principal->alarma);Serial.print("|");
  Serial.print(hw.secundario->alarma);Serial.print("|");

  Serial.println(hw.paro_emergencia);
}

void establece_configuracion(bool guardando_config){

  Serial.print(hw_config.temp_max);Serial.print(" MXT|");
  Serial.print(hw_config.umbral);Serial.print(" UBRL|");
  Serial.print(hw_config.dia_ploteo);Serial.print(" DPLT|");
  Serial.print(hw_config.f_anio);Serial.print(" Y|");
  Serial.print(hw_config.f_mes);Serial.print(" M|");
  Serial.print(hw_config.f_dia);Serial.print(" D|");
  Serial.print(hw_config.f_hora);Serial.print(" H|");
  Serial.print(hw_config.f_min);Serial.println(" min|");

  hw.temp_max = hw_config.temp_max;
  hw.umbral = hw_config.umbral;
  hw.dia_ploteo = hw_config.dia_ploteo;

  if(guardando_config){
    // seconds, minutes, hours, day of the week, day of the month, month, year
    myRTC.setDS1302Time(hw_config.f_seg,hw_config.f_min, hw_config.f_hora, 0, hw_config.f_dia, hw_config.f_mes, hw_config.f_anio);
  }
  
  momento_actual = now();
  // hours, minutes, seconds, day, month, year
  setTime(hour(momento_actual), minute(momento_actual), second(momento_actual),  day(momento_actual), month(momento_actual), year(momento_actual));

  // seconds, minutes, hours, day of the week, day of the month, month, year
  // myRTC.setDS1302Time(0, 2, 3, 4, 10, 9, 2024);
  //       //hh, mm, ss,DD,MM,AAAA
  // setTime(5, 6, 7, 10, 9, 2024);

}
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20);
  
  u8g2.begin();
  u8g2.setContrast(0); // Contraste  ¿
  
  hw_config.carga_eprom();
  myRTC.updateTime();
  establece_configuracion(false);

  // Configurar el pin del botón como entrada (paro de emergencia)
  pinMode(2, INPUT);
    
}

void loop() {
  
  myRTC.updateTime();
  momento_actual = now();

  hw.verifica_alarmas();
  hw.verifica_paro_emergencia();
  hw.verifica_temperaturas();
  hw.verifica_ploteo(day(momento_actual));
  ejecuta_accion_comando(recibir_comando());
  actualiza_display();
}
