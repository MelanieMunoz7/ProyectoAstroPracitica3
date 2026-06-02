#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial maestro(2, 3);
Servo servoH;
Servo servoV;

float targetH = 90.0; 
float targetV = 20.0; 
float currentH = 90.0;
float currentV = 20.0;
float stepSize = 0.02; // Reducido para mayor fluidez sub-grado

String data = "";

// Convierte grados decimales a microsegundos (Resolución alta)
int gradosAMicrosegundos(float grados) {
  // En la librería Servo: 0° = 544 us, 180° = 2400 us
  return 544 + (grados / 180.0) * (2400 - 544);
}

void setup() {
  Serial.begin(9600);
  maestro.begin(9600);
  
  servoH.attach(9);
  servoV.attach(10);
  
  Serial.println("INICIANDO CALIBRACIÓN: Moviendo al Norte...");
  servoH.write(90);
  servoV.write(20);
  
  delay(6000); 
  Serial.println("Calibración completada. Sistema listo.");
}

void loop() {
  // 1. Leer datos del maestro
  while (maestro.available()) {
    char c = maestro.read();
    if (c == '\n') {
      int commaIndex = data.indexOf(',');
      if (commaIndex != -1) {
        targetH = data.substring(0, commaIndex).toFloat(); // Ahora lee decimales
        targetV = data.substring(commaIndex + 1).toFloat(); // Ahora lee decimales
      }
      data = "";
    } else if (c != '\r') { // Ignoramos \r para evitar caracteres basura
      data += c;
    }
  }

  // 2. Lógica de suavizado (Interpolación fina)
  if (abs(targetH - currentH) > 0.02) {
    if (currentH < targetH) currentH += stepSize;
    else currentH -= stepSize;
    servoH.writeMicroseconds(gradosAMicrosegundos(currentH));
  }

  if (abs(targetV - currentV) > 0.02) {
    if (currentV < targetV) currentV += stepSize;
    else currentV -= stepSize;
    servoV.writeMicroseconds(gradosAMicrosegundos(currentV));
  }

  delay(15); 
}
