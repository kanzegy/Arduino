#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include <TimeLib.h>
#include "virtuabotixRTC.h"
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

virtuabotixRTC myRTC(6, 7, 5);
U8G2_ST7565_ERC12864_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=CSL*/ 13, /* data=SI*/ 11, /* cs=CS*/ 10, /* dc=RS*/ 9, /* reset=RSE*/ 8);
time_t momento_actual;

enum direccion {
  arriba = 0,
  abajo = 1,
  atras = 2,
  adelante = 3
};

// Definir la estructura accion_boton
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
    char alias_str[1];

    // Constructor

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
    } 
    // Función para modificar los valores de max y min
    void setMaxMin(int nuevo_max, int nuevo_min) {
        max = nuevo_max;
        min = nuevo_min;
    }

    // Función preciona_arriba: Incrementa el valor de parametro_afectado
    int preciona_arriba() {
      Serial.println("preciona_arriba");
      if (parametro_afectado && parametro_afectado != nullptr && *parametro_afectado < max) {
          (*parametro_afectado)++;
      }
      return arr;
    }

    // Función preciona_abajo: Reduce el valor de parametro_afectado
    int preciona_abajo() {
      Serial.println("preciona_abajo");
      if (parametro_afectado && parametro_afectado != nullptr && *parametro_afectado > min) {
          (*parametro_afectado)--;
      }
      return aba;
    }

    // Función preciona_atras: Retorna el valor de atras
    int preciona_atras() {
      Serial.println("preciona_atras");
      return atr;
    }

    // Función preciona_adelante: Retorna el valor de adelante
    int preciona_adelante() {
      Serial.println("preciona_adelante");
      return ade;
    }
};

// Enum Estados
enum Estados {
  h_apagado = 1,
  h_encendido = 2,

  d_mostrar_temperaturas = 0,
  rare = 999
};

// Estructura Clima
struct Clima {
  int sensor;
  bool es_principal;
  Estados estado;
  time_t principal_desde;
  time_t hora_encendido;
  double temp;

  void apagar() {
    if(estado == h_encendido){
      //Serial.print("apagando clima: ");
      //Serial.println(sensor);
      estado = h_apagado;
    }
  }

  void encender() {
    if(estado == h_apagado){
      //Serial.print("encendiendo clima: ");
      //Serial.println(sensor);
      estado = h_encendido;
      hora_encendido = now();
    }
  }

  double leer_temperatura(){
    int val;
    val = analogRead(sensor);
    temp = log(((10240000/val) - 10000));
    temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp ))* temp );
    temp = temp - 273.15;
    return temp;
  }

  long tiempo_encendido(){
    if (estado == h_apagado) return -1;
    long tiempo = now() - hora_encendido;// Diferencia en segundos
    return tiempo;
  }

  long tiempo_siendo_principal(){
    if (!es_principal) return -1;
    long diferencia = now() - principal_desde;
    return diferencia / 86400;//86400 segundos en un dia
  }
};

struct Config {
  int temp_max;
  int umbral;
  int f_dia;
  int f_mes;
  int f_anio;
  int f_hora;
  int f_min;
  int ploteo_dias;
};
// Estructura estado_hw
struct estado_hw {
  Clima* principal;
  Clima* secundario;
  int lugar_actual; 
  double temp_max;
  double umbral;
  int ploteo_dias;

  void verifica_ploteo() {
    //Serial.println("verificando ploteo");
    
    //Serial.print("principal actual: ");
    //Serial.print(principal->sensor);
    //Serial.print(" | dias restantes: ");
    //Serial.println(7 - principal->tiempo_siendo_principal());

    if(principal->tiempo_siendo_principal() >= ploteo_dias){
      
      //Serial.println("p-* Realizando plot entre climas: ");
      // Intercambio entre principal y secundario
      Clima* temp = principal;
      principal = secundario;
      secundario = temp;

      // Cambiar el valor de es_principal
      principal->es_principal = true;
      principal->principal_desde = now();
      
      secundario->es_principal = false;

      // Mostrar los cambios en la consola
      //Serial.print("p-* nuevo principal: ");
      //Serial.println(principal->sensor);
      //Serial.print("p-* nuevo secundario: ");
      //Serial.println(secundario->sensor);
    }
  }

  void verifica_temperaturas(){
    //Serial.println("checando temperaturas");
    principal->leer_temperatura(); //actualizamos el valor de la temperatura
    secundario->leer_temperatura();
    bool max_revasado = (principal->temp > temp_max || secundario->temp > temp_max);
    bool min_alcanzado = (principal->temp < (temp_max - umbral) || secundario->temp < (temp_max - umbral));
    if(min_alcanzado){
      //Serial.println("t-- min alcanzado, apagando...");
        principal->apagar();
        secundario->apagar();
    }
    else if(max_revasado){
      //Serial.println("t-- maximo revazado");
      if(principal->estado == h_apagado && (secundario->estado == h_apagado || secundario->tiempo_encendido() > 5*60/*5 MiNUTOS*/)){
        //Serial.println("t-- encendiendo principal");
        principal->encender();
      }
      else if(principal->tiempo_encendido() > 5*60/*5 MiNUTOS*/){
        //Serial.println("t-- encendiendo secundario...");
        secundario->encender();
      }

      // //Serial.print("t-- principal encendido durante (segundos): ");
      // //Serial.print(principal->tiempo_encendido());
      // if(principal->estado == h_apagado) //Serial.println("(OFF)");
      // if(principal->estado == h_encendido) //Serial.println("(ON)");
      
      // //Serial.print("t-- secundario encendido durante (segundos): ");
      // //Serial.print(secundario->tiempo_encendido());
      // if(secundario->estado == h_apagado) //Serial.println("(OFF)");
      // if(secundario->estado == h_encendido) //Serial.println("(ON)");
    }
    
  }
};
char alias_actual = 'W';
Clima clima_principal;
Clima clima_secundario;
estado_hw hw = {&clima_principal, &clima_secundario, 0, 25.0, 22.0, 7};
Config hw_config = {0,0,0,0,0,0,0,0};

accion_boton  lugares[15] = {
  // accion_boton(x, y, larg, AR, AB, AT, AD, MAX, MIN, VARIABLE A CONTROLAR, ALIAS), // EJEMPLO
  accion_boton(0, 0, 0, -1, -1, -1, 1, 0, 0, nullptr, 'D'), // DASHBOARD - 0
  accion_boton(10, 1, 48, 6, 2, 0, 7, 0, 0, nullptr, 'T'), // TEMPERATURA - 1
  accion_boton(10, 11, 48, 1, 3, 0, 8, 0, 0, nullptr, 'U'), // UMBRAL - 2
  accion_boton(10, 21, 48, 2, 4, 0, 9, 0, 0, nullptr, 'F'), // FECHA - 3
  accion_boton(10, 31, 48, 3, 5, 0, 12, 0, 0, nullptr, 'H'), // HORA - 4
  accion_boton(10, 41, 48, 4, 6, 0, 14, 0, 0, nullptr, 'P'), // PLOTEO - 5
  accion_boton(43, 53, 42, 5, 1, 0, 100, 0, 0, nullptr, 'A'), // ACEPTAR - 6
  accion_boton(68, 1, 41, -1, -1, 1, -1, 30, 22, &hw_config.temp_max, 't'), // CONFIG TEMPERATURA - 7
  accion_boton(68, 11, 41, -1, -1, 2, -1, 3, 15, &hw_config.umbral, 'u'), // CONFIG UMBRAL - 8
  accion_boton(68, 21, 11, -1, -1, 3, 10, 30, 0, &hw_config.f_dia, 'd'), // CONFIG DIA - 9
  accion_boton(86, 21, 11, -1, -1, 9, 11, 12, 0, &hw_config.f_mes, 'm'), // CONFIG MES - 10
  accion_boton(104, 21, 22, -1, -1, 10, -1, 9999, 1992, &hw_config.f_anio, 'a'), // CONFIG AÑO - 11
  accion_boton(68, 31, 12, -1, -1, 4, 13, 23, 0, &hw_config.f_hora, 'h'), // CONFIG HORA - 12
  accion_boton(86, 31, 12, -1, -1, 12, -1, 59, 0, &hw_config.f_min, 'i'), // CONFIG MINUTO - 13
  accion_boton(68, 41, 41, -1, -1, 5, -1, 30, 5, &hw_config.ploteo_dias, 'p'), // CONFIG PLOTEO - 14
};

direccion recibir_comando() {
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
    }
  }
  return -1; // Retornar -1 si no se recibe un comando válido
}

void ejecuta_accion_comando(direccion dir){
  int next = -1;
  switch(dir){
    case arriba: next = lugares[hw.lugar_actual].preciona_arriba(); break;
    case abajo: next = lugares[hw.lugar_actual].preciona_abajo(); break;
    case atras: next = lugares[hw.lugar_actual].preciona_atras(); break;
    case adelante: next = lugares[hw.lugar_actual].preciona_adelante(); break;
  }
  if(next != -1) {
    Serial.print("next != -1 --> ");
    Serial.println(next);
    hw.lugar_actual = next;
    }
}

void actualiza_display(){
  
  if(hw.lugar_actual == 0){
    muestra_temperaturas();
  }
  else{
    muestra_config();
  }
}

void muestra_config(){
  int ab = 10; //altura base
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_mr);
    
    if(alias_actual != lugares[hw.lugar_actual].alias){
      Serial.println("");
      alias_actual = lugares[hw.lugar_actual].alias;
      Serial.print("|x:");
      Serial.print(lugares[hw.lugar_actual].x);
      Serial.print("|y:");
      Serial.print(lugares[hw.lugar_actual].y);
      Serial.print("|l:");
      Serial.print(lugares[hw.lugar_actual].largo);
      Serial.print("|pa:");
      Serial.print(*lugares[hw.lugar_actual].parametro_afectado);
      Serial.print("|alias -> ");
      Serial.println(lugares[hw.lugar_actual].alias);


      Serial.print("|ARR: ");
      Serial.print(lugares[hw.lugar_actual].arr);
      Serial.print(" - ");
      Serial.println(lugares[lugares[hw.lugar_actual].arr].alias);
      
      Serial.print("|ABA: ");
      Serial.print(lugares[hw.lugar_actual].aba);
      Serial.print(" - ");
      Serial.println(lugares[lugares[hw.lugar_actual].aba].alias);
      
      Serial.print("|ATR: ");
      Serial.print(lugares[hw.lugar_actual].atr);
      Serial.print(" - ");
      Serial.println(lugares[lugares[hw.lugar_actual].atr].alias);

      Serial.print("|ADE: ");
      Serial.print(lugares[hw.lugar_actual].ade);
      Serial.print(" - ");
      Serial.println(lugares[lugares[hw.lugar_actual].ade].alias);
    }

    u8g2.drawStr(10, ab, "Temp max");
    u8g2.drawStr(60, ab, ":");
    u8g2.drawStr(68, ab, _2_string(hw_config.temp_max));

    u8g2.drawStr(10, ab + 10, "Umbral");
    u8g2.drawStr(60, ab + 10, ":");
    u8g2.drawStr(68, ab + 10, _2_string(hw_config.umbral));
    
    u8g2.drawStr(10, ab + 20, "Fecha");
    u8g2.drawStr(60, ab + 20, ":");
    u8g2.drawStr(68, ab + 20,"  /  /");
    u8g2.drawStr(68, ab + 20,_2_digit_number(hw_config.f_dia));
    u8g2.drawStr(86, ab + 20,_2_digit_number(hw_config.f_mes));
    u8g2.drawStr(104,ab + 20,_2_string(hw_config.f_anio));

    u8g2.drawStr(10, ab + 30, "Hora");
    u8g2.drawStr(60, ab + 30, ":");
    u8g2.drawStr(68, ab + 30,_2_digit_number(hw_config.f_hora));
    u8g2.drawStr(86, ab + 30,_2_digit_number(hw_config.f_min));
    
    u8g2.drawStr(10, ab + 40, "Ploteo");
    u8g2.drawStr(60, ab + 40, ":");
    u8g2.drawStr(68, ab + 40, _2_string(hw_config.ploteo_dias));
    u8g2.drawStr(68, ab + 40, _2_string(hw_config.ploteo_dias));
    u8g2.drawStr(77, ab + 40, "dias");
    
    u8g2.drawStr(43, ab + 52, "ACEPTAR");
    
    u8g2.drawHLine(lugares[hw.lugar_actual].x,ab + lugares[hw.lugar_actual].y,lugares[hw.lugar_actual].largo);

  } while ( u8g2.nextPage() );

}
void muestra_temperaturas(){
  u8g2.firstPage();
  int ab = 8; //altura base
  do {
    u8g2.setFont(u8g2_font_6x10_mr);
    
    
    u8g2.drawHLine(0,ab + 2,128);
    u8g2.drawHLine(0,ab + 26,128);
    u8g2.drawStr(0,ab,"Comp   Tmp   ON   Pri");
    u8g2.drawVLine(32,0,34);
    u8g2.drawVLine(68,0,34);
    u8g2.drawVLine(98,0,34);

    u8g2.drawStr(12,ab + 12,_2_string(hw.principal->sensor));
    u8g2.drawStr(38,ab + 12,_2_string(hw.principal->temp));
    u8g2.drawStr(78,ab + 12,(hw.principal->estado == h_encendido ? "si": "-"));
    u8g2.drawStr(108,ab + 12,(hw.principal->es_principal? "si": "-"));

    u8g2.drawStr(12,ab + 22,_2_string(hw.secundario->sensor));
    u8g2.drawStr(38,ab + 22,_2_string(hw.secundario->temp)); 
    u8g2.drawStr(78,ab + 22,(hw.secundario->estado == h_encendido ? "si": "-")); 
    u8g2.drawStr(108,ab + 22,(hw.secundario->es_principal? "si": "-"));

    u8g2.drawStr(0,ab + 36,"MODE:   | ALARMS:");
    u8g2.drawStr(30,ab + 36,"3");
    u8g2.drawStr(105,ab + 36,"NO");
    
    u8g2.drawStr(0,ab + 55,"  /  /       :  :  ");
    u8g2.drawStr(0,ab + 55,_2_digit_number(day(momento_actual)));
    u8g2.drawStr(18,ab + 55,_2_digit_number(month(momento_actual)));
    u8g2.drawStr(36,ab + 55,_2_string(year(momento_actual)));
    u8g2.drawStr(66,ab + 55,_2_digit_number(hour(momento_actual)));
    u8g2.drawStr(83,ab + 55,_2_digit_number(minute(momento_actual)));
    u8g2.drawStr(102,ab + 55,_2_digit_number(second(momento_actual)));

  } while ( u8g2.nextPage() );
}

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  u8g2.setContrast (10); //contraste  
  myRTC.setDS1302Time(0, 0, 0, 2, 16, 9, 2024);
  setTime(7, 8, 9, 10, 9, 2024);

  clima_principal = {1, true, h_apagado, now()};
  clima_secundario = {2, false, h_apagado, now()};
}

void loop() {
    myRTC.updateTime();
    momento_actual = now();
    hw.verifica_temperaturas();
    hw.verifica_ploteo();
    ejecuta_accion_comando(recibir_comando());
    actualiza_display();
}

//funcion para convertir enteros a string
char* _2_string(double numero) {
  static char resultado[20]; // Creamos un buffer para almacenar el string
  dtostrf(numero, 2, 1, resultado); // Convierte el double en string
  return resultado; // Retorna el puntero al string
}
//funcion para convertir doubles a string
char* _2_string(int numero) {
  static char resultado[12]; // Creamos un buffer para almacenar el string (suficiente para int)
  itoa(numero, resultado, 10); // Convierte el entero en string en base 10
  return resultado; // Retorna el puntero al string
}

//funcion para convertir enteros a string con valor del 0 al 99 y se agrega un cero al inicio cuando son numeros de 1 digito
char* _2_digit_number(int valor){
  if( valor > 9){
    return _2_string(valor);
  }
  else{
    char* data = _2_string(valor);
    data[2] = data[1]; //se pasa el salto de linea de la primera a la tercer posicion del array
    data[1] = data[0]; //se pasa el valor del digito convertido de la primera a la segunda posicion del array
    data[0] = '0'; // se agrega el cero en la primera posicion del array
    return data;
  }
}
