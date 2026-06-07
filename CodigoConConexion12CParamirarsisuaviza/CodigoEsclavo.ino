#include <Wire.h>
#include <Servo.h>

Servo servoH;
Servo servoV;

float targetH = 90.0; 
float targetV = 20.0; 
float currentH = 90.0;
float currentV = 20.0;
float stepSize = 0.02; 

//Misma estructura para reconstruir el float a partir de los bytes recibidos
union FloatALongitud 
{
  float valorFloat;
  byte bytes[4];
};

int gradosAMicrosegundos(float grados) 
{
  return 544 + (grados / 180.0) * (2400 - 544);
}

void setup() {
  Serial.begin(9600);
  
  Wire.begin(8);                
  Wire.onReceive(recibirDatos); 
  
  servoH.attach(9);
  servoV.attach(10);
  
  Serial.println("INICIANDO CALIBRACIÓN: Moviendo al Norte...");
  servoH.write(90);
  servoV.write(20);
  delay(6000); 
  Serial.println("Sistema listo.");
}

void loop() 
{
  //LÓGICA DE SUAVIZADO (Interpolación ultra-fina con destinos decimales)
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

//Se ejecuta al recibir los 8 bytes del maestro
void recibirDatos(int howMany) 
{
  // Esperamos exactamente 8 bytes (4 para H y 4 para V)
  if (howMany >= 8) {
    FloatALongitud datoH;
    FloatALongitud datoV;

    // Leer los primeros 4 bytes para el servo Horizontal
    for (int i = 0; i < 4; i++) {
      datoH.bytes[i] = Wire.read();
    }
    
    // Leer los siguientes 4 bytes para el servo Vertical
    for (int i = 0; i < 4; i++) {
      datoV.bytes[i] = Wire.read();
    }

    // Asignar los valores float reconstruidos
    targetH = datoH.valorFloat;
    targetV = datoV.valorFloat;
  }
}
