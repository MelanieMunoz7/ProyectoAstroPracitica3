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
  Serial.begin(9600);
  esclavo.begin(9600);

  Wire.begin();
  rtc.begin();

  // SOLO UNA VEZ para ajustar el RTC
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  // -------- LEER HORA --------
  DateTime now = rtc.now();
  int n = now.dayOfYear();
  float hour = now.hour() + now.minute() / 60.0 + now.second() / 3600.0;

  // -------- MODELO SOLAR --------
  float delta = 23.45 * sin((360.0 / 365.0) * (n - 81) * PI / 180);
  delta *= PI / 180;

  float H = 15 * (hour - 12);
  H *= PI / 180;

  float sin_alpha = sin(lat) * sin(delta) + cos(lat) * cos(delta) * cos(H);
  float alpha = asin(sin_alpha);

  float cosA = (sin(delta) - sin(alpha) * sin(lat)) / (cos(alpha) * cos(lat));
  float A = acos(cosA);

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

  // -------- MOSTRAR --------
  Serial.print("Hora: ");
  Serial.print(hour);
  Serial.print("  Luz: ");
  Serial.print(light);
  Serial.print("  Azim: ");
  Serial.print(azim);
  Serial.print("  Elev: ");
  Serial.println(elev);

  // -------- ENVIAR AL ESCLAVO --------
  esclavo.print(posH);
  esclavo.print(",");
  esclavo.println(posV);

  delay(2000);
}
