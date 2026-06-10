#include <SoftwareSerial.h>
#include "RTClib.h"
#include <math.h>

SoftwareSerial puertoEsclavo(2, 3); 

RTC_DS1307 rtc;
int sensor = A0;
float lat = 6.24 * PI / 180; // Latitud de Medellín en radianes

// Función para mapear con precisión decimal
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)  {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
  Serial.begin(9600);   
  puertoEsclavo.begin(9600);  
  
  if (!rtc.begin()) {
    Serial.println("No se encuentra el RTC");
  }
  
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Descomentar solo si necesitas resincronizar hora
}

void loop() {
  // -------- 1. LECTURA DEL TIEMPO (RTC) --------
  DateTime now = rtc.now();
  int diasMes[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int n = diasMes[now.month() - 1] + now.day();
  float hourDecimal = now.hour() + now.minute() / 60.0 + now.second() / 3600.0;

  // -------- 2. CÁLCULO ASTRONÓMICO SOLAR (CORREGIDO) --------
  float gamma = (360.0 / 365.0) * (n - 81) * PI / 180.0;
  float delta = 23.45 * PI / 180.0 * sin(gamma); // Declinación en radianes
  float H = 15.0 * (hourDecimal - 12.0) * PI / 180.0; // Ángulo horario en radianes

  // Elevación (Altura)
  float sin_alpha = sin(lat) * sin(delta) + cos(lat) * cos(delta) * cos(H);
  sin_alpha = constrain(sin_alpha, -1.0, 1.0); 
  float alpha = asin(sin_alpha); // Altura en radianes

  // Azimut usando atan2 para evitar errores de cuadrante (Mañana/Tarde)
  float y = -sin(H) * cos(delta);
  float x = sin(lat) * cos(delta) * cos(H) - cos(lat) * sin(delta);
  float A = atan2(y, x); // Azimut en radianes

  // Conversión a grados reales del Sol
  float elev = alpha * 180.0 / PI;
  float azim = A * 180.0 / PI;

  // Ajustar Azimut en rango de 0° a 360°
  if (azim < 0) {
    azim = 360.0 + azim;
  }

  // -------- 3. TRADUCCIÓN A ÁNGULOS DE SERVO --------
  float posH;
  if (azim <= 180) {
    posH = mapFloat(azim, 0.0, 180.0, 90.0, 160.0); 
  } else {
    posH = mapFloat(azim, 180.0, 360.0, 20.0, 90.0);   
  }
  float posV = mapFloat(elev, 0.0, 90.0, 20.0, 160.0);

  // Límites de seguridad física para los servos
  posH = constrain(posH, 20.0, 160.0);
  posV = constrain(posV, 20.0, 160.0);
  
  if (elev < 0) { // Modo nocturno (retorno al origen)
    posH = 90.0; 
    posV = 20.0; 
  }

  // -------- 4. CÁLCULO DE IRRADIANCIA --------
  const float RL = 1000.0;        
  const float RESPONSIVIDAD = 0.54; 
  const float AREA_M2 = 1.37e-5;    
  
  int lectura = analogRead(sensor); 
  float voltaje = (1023.0 - (float)lectura) * (5.0 / 1023.0);
  float corriente = voltaje / RL;
  float potenciaW = corriente / RESPONSIVIDAD;
  float irradiancia = potenciaW / AREA_M2;
  
  // -------- 5. SALIDA AL MONITOR SERIE --------
  Serial.print("Hora: ");
  if(now.hour() < 10) Serial.print('0');
  Serial.print(now.hour()); Serial.print(':');
  if(now.minute() < 10) Serial.print('0');
  Serial.print(now.minute()); Serial.print(':');
  if(now.second() < 10) Serial.print('0');
  Serial.print(now.second());
  
  Serial.print(" | Irradiancia: "); Serial.print(irradiancia, 2); Serial.print(" W/m2");
  Serial.print(" | Azimut Sol: "); Serial.print(azim, 2); Serial.print("°");
  Serial.print(" | Altura Sol: "); Serial.print(elev, 2); Serial.println("°");


  // -------- 6. ENVÍO DE DATOS ENCAPSULADOS AL ESCLAVO --------
  puertoEsclavo.print("<");
  puertoEsclavo.print(posH, 2);
  puertoEsclavo.print(",");
  puertoEsclavo.print(posV, 2);
  puertoEsclavo.println(">");

  delay(20000); 
}
