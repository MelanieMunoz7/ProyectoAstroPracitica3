#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial maestro(2, 3);
Servo servoH;
Servo servoV;

// Variables para suavizado
int targetH = 90;
int targetV = 90;
float currentH = 90;
float currentV = 90;
float stepSize = 0.05; // AJUSTA ESTO: Menor número = más lento/suave

String data = "";

void setup() {
  Serial.begin(9600);
  maestro.begin(9600);
  
  servoH.attach(9);
  servoV.attach(10);
  
  servoH.write(currentH);
  servoV.write(currentV);
}

void loop() {
  // 1. Leer datos del maestro
  while (maestro.available()) {
    char c = maestro.read();
    if (c == '\n') {
      int commaIndex = data.indexOf(',');
      if (commaIndex != -1) {
        targetH = data.substring(0, commaIndex).toInt();
        targetV = data.substring(commaIndex + 1).toInt();
      }
      data = "";
    } else {
      data += c;
    }
  }

  // 2. Lógica de suavizado (Movimiento incremental)
  // Movimiento Horizontal
  if (abs(targetH - currentH) > 0.1) {
    if (currentH < targetH) currentH += stepSize;
    else currentH -= stepSize;
    servoH.write((int)currentH);
  }

  // Movimiento Vertical
  if (abs(targetV - currentV) > 0.1) {
    if (currentV < targetV) currentV += stepSize;
    else currentV -= stepSize;
    servoV.write((int)currentV);
  }

  delay(15); // Pequeña pausa para dar tiempo al servo y controlar la velocidad total
}
