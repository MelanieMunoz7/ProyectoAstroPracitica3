#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial maestro(2, 3);
Servo servoH;
Servo servoV;

// Posiciones iniciales de calibración (Fijadas en el Norte)
int targetH = 90; // 90° será físicamente el Norte magnético/geográfico
int targetV = 20; // Horizonte bajo
float currentH = 90;
float currentV = 20;
float stepSize = 0.15; // Un poco más rápido para responder ágilmente al arrancar

String data = "";

void setup() {
  Serial.begin(9600);
  maestro.begin(9600);
  
  servoH.attach(9);
  servoV.attach(10);
  
  // === RUTINA DE CENTRADO AL NORTE ===
  Serial.println("INICIANDO CALIBRACIÓN: Moviendo al Norte...");
  servoH.write(targetH);
  servoV.write(targetV);
  
  // Bloqueo de 6 segundos para que acomodes la estructura físicamente hacia el norte con una brújula
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
        targetH = data.substring(0, commaIndex).toInt();
        targetV = data.substring(commaIndex + 1).toInt();
      }
      data = "";
    } else {
      data += c;
    }
  }

  // 2. Lógica de suavizado (Movimiento incremental)
  if (abs(targetH - currentH) > 0.5) {
    if (currentH < targetH) currentH += stepSize;
    else currentH -= stepSize;
    servoH.write((int)currentH);
  }

  if (abs(targetV - currentV) > 0.5) {
    if (currentV < targetV) currentV += stepSize;
    else currentV -= stepSize;
    servoV.write((int)currentV);
  }

  delay(15); 
}
