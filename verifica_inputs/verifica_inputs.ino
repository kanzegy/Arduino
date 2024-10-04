const int pinInicio = 3; // Ignorar pines 1 y 2
const int pinFin = 13; // Hasta el pin 13 (ajustar según tu placa)
int estadosAnteriores[14]; // Almacenar los estados anteriores de los pines (3 al 13)

void setup() {
  Serial.begin(9600); // Inicializar comunicación serial

  // Configurar los pines como entradas
  for (int pin = pinInicio; pin <= pinFin; pin++) {
    pinMode(pin, INPUT);
    estadosAnteriores[pin] = digitalRead(pin); // Leer el estado inicial
  }
}

void loop() {
  for (int pin = pinInicio; pin <= pinFin; pin++) {
    int estadoActual = digitalRead(pin); // Leer el estado actual

    // Si el estado del pin ha cambiado, enviar una notificación por el puerto serial
    if (estadoActual != estadosAnteriores[pin]) {
      Serial.print("El pin ");
      Serial.print(pin);
      Serial.println(" ha cambiado de valor.");
      
      estadosAnteriores[pin] = estadoActual; // Actualizar el estado anterior
    }
  }
  delay(100); // Pequeña pausa para evitar excesivas lecturas
}
