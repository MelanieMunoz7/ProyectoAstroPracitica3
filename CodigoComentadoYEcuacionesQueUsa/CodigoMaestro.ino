#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include <math.h>

// Comunicación con el Arduino Esclavo (RX, TX)
SoftwareSerial esclavo(2, 3);

RTC_DS1307 rtc;

// Fotodiodo
int sensor = A0;

// Medellín
float lat = 6.24 * PI / 180;

void setup() {
  Serial.begin(9600);   // Monitor Serial de la PC
  esclavo.begin(9600);  // Comunicación con el otro Arduino
  Wire.begin();
  
  if (!rtc.begin()) {
    Serial.println("No se encuentra el RTC");
  }
  
  // Ajusta la hora si es necesario
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  // -------- LEER HORA --------
  DateTime now = rtc.now();

  int diasMes[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int n = diasMes[now.month() - 1] + now.day();

  float hourDecimal = now.hour() + now.minute() / 60.0 + now.second() / 3600.0;

  // -------- MODELO SOLAR/LUNAR --------
  float delta = 23.45 * sin((360.0 / 365.0) * (n - 81) * PI / 180);
  delta *= PI / 180;

  float H = 15 * (hourDecimal - 12);
  H *= PI / 180;

  float sin_alpha = sin(lat) * sin(delta) + cos(lat) * cos(delta) * cos(H);
  float alpha = asin(sin_alpha);

  float cosA = (sin(delta) - sin(alpha) * sin(lat)) / (cos(alpha) * cos(lat));
  float A = acos(constrain(cosA, -1.0, 1.0)); // Constrain para evitar errores matemáticos

  // -------- GRADOS --------
  float elev = alpha * 180 / PI;
  float azim = A * 180 / PI;

  // -------- MAPEO A SERVOS --------
  int posV = map(elev, 0, 90, 20, 160);
  int posH = map(azim, 0, 180, 20, 160);

  posH = constrain(posH, 20, 160);
  posV = constrain(posV, 20, 160);

  // -------- LEER SENSOR --------
  int light = analogRead(sensor);

  // -------- MOSTRAR EN MONITOR SERIAL (PC) --------
  Serial.print("Hora: ");
  if(now.hour() < 10) Serial.print('0');
  Serial.print(now.hour()); Serial.print(':');
  if(now.minute() < 10) Serial.print('0');
  Serial.print(now.minute());
  
  Serial.print(" | Luz: ");
  Serial.print(light);
  
  Serial.print(" | Azim: ");
  Serial.print(azim);
  
  Serial.print(" | Elev: ");
  Serial.println(elev);

  // -------- ENVIAR AL ESCLAVO --------
  // Enviamos los datos en una sola línea para que el esclavo los procese
  esclavo.print(posH);
  esclavo.print(",");
  esclavo.println(posV);

  delay(2000); // Actualiza cada 2 segundos
}
