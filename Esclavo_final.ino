#include <SoftwareSerial.h>
#include <Servo.h>

//Configuración de pines para comunicación: RX = 2, TX = 3
SoftwareSerial puertoMaestro(2, 3);

Servo servoH;
Servo servoV;

//Variables de posición con decimales
float targetH = 90.0; 
float targetV = 20.0; 
float currentH = 90.0;
float currentV = 20.0;

//Tamaño del paso para el movimiento lento (0.02 grados)
float stepSize = 0.02; 

//Conversión de alta precisión: Grados flotantes a Microsegundos de pulso
int gradosAMicrosegundos(float grados) {
  return 544 + (grados / 180.0) * (2400 - 544);
}

void setup() {
  puertoMaestro.begin(9600);
  
  servoH.attach(9);
  servoV.attach(10);
  
  //Ubicación inicial suave al arrancar
  servoH.write(90);
  servoV.write(20);
  delay(2000); 
}

void loop() {
  // -------- 1. RECEPCIÓN Y DESEMPAQUETADO DE DATOS --------
  if (puertoMaestro.available() > 0) {
    char inicio = puertoMaestro.read();
    if (inicio == '<') { // Detecta el inicio seguro del mensaje
      targetH = puertoMaestro.parseFloat();
      targetV = puertoMaestro.parseFloat();
    }
  }

  // -------- 2. RAMPA DE SUAVIZADO HORIZONTAL --------
  if (abs(targetH - currentH) > 0.02) {
    if (currentH < targetH) currentH += stepSize;
    else currentH -= stepSize;
    servoH.writeMicroseconds(gradosAMicrosegundos(currentH));
  }

  // -------- 3. RAMPA DE SUAVIZADO VERTICAL --------
  if (abs(targetV - currentV) > 0.02) {
    if (currentV < targetV) currentV += stepSize;
    else currentV -= stepSize;
    servoV.writeMicroseconds(gradosAMicrosegundos(currentV));
  }

  delay(15); // Sincronización del pulso del servo (15ms standard)
}
