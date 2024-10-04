#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  
  // Borrar la EEPROM escribiendo 0xFF en todas las direcciones
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0xFF);  // Escribir 0xFF en cada dirección
  }

  Serial.println("EEPROM borrada");
}

void loop() {
  // No se necesita código en el loop para esta operación
}
