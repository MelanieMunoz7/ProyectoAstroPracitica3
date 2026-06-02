#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include <math.h>

SoftwareSerial esclavo(2, 3);
RTC_DS1307 rtc;

int sensor = A0;
float lat = 6.24 * PI / 180; // Medellín en radianes

// Función map para números decimales
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
  Serial.begin(9600);   
  esclavo.begin(9600);  
  Wire.begin();
  
  if (!rtc.begin()) {
    Serial.println("No se encuentra el RTC");
  }
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Descomentar solo para setear la hora inicial
}

void loop() {
  // -------- LEER HORA --------
  DateTime now = rtc.now();

  int diasMes[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int n = diasMes[now.month() - 1] + now.day();

  float hourDecimal = now.hour() + now.minute() / 60.0 + now.second() / 3600.0;

  // -------- MODELO SOLAR --------
  float delta = 23.45 * sin((360.0 / 365.0) * (n - 81) * PI / 180);
  delta *= PI / 180;

  float H = 15 * (hourDecimal - 12);
  H *= PI / 180;

  float sin_alpha = sin(lat) * sin(delta) + cos(lat) * cos(delta) * cos(H);
  float alpha = asin(sin_alpha);

  float cosA = (sin(delta) - sin_alpha * sin(lat)) / (cos(alpha) * cos(lat));
  float A = acos(constrain(cosA, -1.0, 1.0)); 

  float elev = alpha * 180 / PI;
  float azim = A * 180 / PI;

  if (H > 0) {
    azim = 360.0 - azim;
  }

  // -------- MAPEO A SERVOS (Con decimales) --------
  float posH, posV;
  
  if (azim <= 180) {
    posH = mapFloat(azim, 0, 180, 90, 160); 
  } else {
    posH = mapFloat(azim, 180, 360, 20, 90);  
  }

  posV = mapFloat(elev, 0, 90, 20, 160);

  // Filtros de seguridad física
  posH = constrain(posH, 20.0, 160.0);
  posV = constrain(posV, 20.0, 160.0);

  if (elev < 0) {
    posH = 90.0; 
    posV = 20.0; 
  }

  // -------- CÁLCULO DE IRRADIANCIA --------
  const float RL = 100000.0;        
  const float RESPONSIVIDAD = 0.62; 
  const float AREA_M2 = 1.37e-5;    
  
  int lectura = analogRead(sensor); 
  float voltaje = lectura * (5.0 / 1023.0);
  float corriente = voltaje / RL;
  float potenciaW = corriente / RESPONSIVIDAD;
  float irradiancia = potenciaW / AREA_M2;
  
  // -------- ENVIAR AL ESCLAVO --------
  // Al enviar un float, Arduino envía 2 decimales por defecto (ej. "90.15")
  esclavo.print(posH);
  esclavo.print(",");
  esclavo.println(posV);

  delay(2000); 
}
