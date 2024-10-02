#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include "virtuabotixRTC.h"
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
//#ifdef U8X8_HAVE_HW_I2C
//#include <Wire.h>
//#endif
// Definimos el enum llamado "estados"
enum estados {
  iniciando = 00,
  encender_principal = 02,
  encender_secundario = 01,

  mostrar_temperaturas = 10,
  config_limit_max = 11,
  config_limit_min = 12,

  rare = 999
};

// Creamos un tipo basado en el enum
typedef enum estados Estado;

// Definimos la estructura que contiene las variables solicitadas
struct Configuracion {
  double min_limit;
  double max_limit;
  double temps[2];
  int aire_principal;
  int cantidad_aires;

  Estado estado_menus;
  Estado nuevo_estado_m;
  
  Estado estado_funcional;
  Estado nuevo_estado_f;
};
Configuracion conf; 

virtuabotixRTC myRTC(6, 7, 5);
U8G2_ST7565_ERC12864_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=CSL*/ 13, /* data=SI*/ 11, /* cs=CS*/ 10, /* dc=RS*/ 9, /* reset=RSE*/ 8);



void setup(void) {
  conf.min_limit = 22.0;
  conf.max_limit = 25.0;
  conf.aire_principal = 1;
  conf.cantidad_aires = 2;
  conf.estado_menus = mostrar_temperaturas;
  conf.nuevo_estado_m = mostrar_temperaturas;
  conf.estado_funcional = iniciando;
  conf.nuevo_estado_f = iniciando;

  Serial.begin(9600);
  //myRTC.setDS1302Time(0, 24, 14, 4, 15, 9, 2024); //Configuración del tiempo actual
  u8g2.begin();
  u8g2.setContrast (10); //contraste  
  myRTC.setDS1302Time(00, 59, 23, 6, 10, 1, 2014);
}

void loop(void) {
 
  for (int i = 0; i < conf.cantidad_aires; i++) {
    conf.temps[i] = obtener_temperatura(i);

    if(conf.temps[i] < conf.min_limit || conf.max_limit < conf.temps[i]){
      conf.nuevo_estado_f = encender_principal;
    }
  }
  
  myRTC.updateTime();

  //dtostrf(year1, 5, 0, myRTC.year); //turn your floats into strings. 
//***********************
  u8g2.firstPage();
  do {
    //u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setFont(u8g2_font_6x10_mr);
    u8g2.drawStr(0,12,"Temp1: ");
    u8g2.drawStr(36,12,_2_string(conf.temps[0]));
   
    //u8g2.drawStr(0,12,"Temp1: 30.3");
    u8g2.drawStr(0,24,"Temp2:");
    u8g2.drawStr(36,24,_2_string(conf.temps[1]));
         
    u8g2.drawStr(0,36,"Comp1: ON");
    u8g2.drawStr(0,48,"Comp2: OFF");
    //u8g2.drawStr(0,60,"26-09-24, 12:30");
    u8g2.drawStr(0,60,"201124");
    u8g2.drawStr(80,12,"OpMo: 3");
    u8g2.drawStr(80,24,"Dly : 15");
    u8g2.drawStr(80,36,"  NO");
    u8g2.drawStr(80,48,"ALARMS");

   
  } while ( u8g2.nextPage() );
  delay(1000);
}

void ejecutar_estados(){
  switch(conf.nuevo_estado_f){
    case iniciando: conf.estado_funcional = encender_principal;break;
    case encender_principal: 
      switch(conf.estado_funcional){
        case iniciando: conf.estado_funcional = encender_principal; break;
        case encender_principal: conf.estado_funcional = encender_secundario; break;
        case encender_secundario: /*?????*/ break;
      }
    break;
    case encender_secundario:
      switch(conf.estado_funcional){
        case iniciando: conf.estado_funcional = encender_principal; break;
        case encender_principal: conf.estado_funcional = encender_secundario; break;
        case encender_secundario: /*?????*/ break;
      }
    break;
  }
}

double obtener_temperatura(int sensor){
  int val;
  double temp;
  val = analogRead(sensor);
  temp = log(((10240000/val) - 10000));
  temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp ))* temp );
  return temp = temp - 273.15;
}

char* _2_string(double numero) {
  static char resultado[20]; // Creamos un buffer para almacenar el string
  dtostrf(numero, 6, 2, resultado); // Convierte el double en string
  return resultado; // Retorna el puntero al string
}
char* _2_string(int numero) {
  static char resultado[12]; // Creamos un buffer para almacenar el string (suficiente para int)
  itoa(numero, resultado, 10); // Convierte el entero en string en base 10
  return resultado; // Retorna el puntero al string
}

void enviar_serial(char* temp1, char* temp2){
  Serial.print(temp1);
  Serial.println("°C");

    Serial.print(temp2);
  Serial.println("°C");
}