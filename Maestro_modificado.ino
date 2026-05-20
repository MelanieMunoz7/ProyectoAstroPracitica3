#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include <math.h>

SoftwareSerial esclavo(2, 3);
RTC_DS1307 rtc;

int sensor = A0;
float lat = 6.24 * PI / 180; // Medellín en radianes

void setup() {
  Serial.begin(9600);   
  esclavo.begin(9600);  
  Wire.begin();
  
  if (!rtc.begin()) {
    Serial.println("No se encuentra el RTC");
  }
  
  // ¡OJO! Al dejarlo fijo por días, comenta esta línea tras la primera carga 
  // para que no se resetee la hora exacta si el Arduino se reinicia solo.
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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

  float cosA = (sin(delta) - sin(alpha) * sin(lat)) / (cos(alpha) * cos(lat));
  float A = acos(constrain(cosA, -1.0, 1.0)); 

  // Conversión a grados aritméticos iniciales
  float elev = alpha * 180 / PI;
  float azim = A * 180 / PI;

  // CORRECCIÓN ASTRONÓMICA: Si es de tarde (H > 0), el Azimut real supera los 180°
  if (H > 0) {
    azim = 360.0 - azim;
  }

  // -------- MAPEO A SERVOS (Referencia: Norte = 90°) --------
  int posH;
  
  // Mapeamos el comportamiento diurno para un servo de 180 grados:
  // Mañana: El sol viene desde el Este (~90° de Azimut) hacia el Norte (0°/360°)
  if (azim <= 180) {
    posH = map(azim, 0, 180, 90, 160); // 90 es Norte, 160 tiende al Este
  } 
  // Tarde: El sol va desde el Norte (0°/360°) hacia el Oeste (~270° de Azimut)
  else {
    posH = map(azim, 180, 360, 20, 90);  // 20 tiende al Oeste, 90 es Norte
  }

  // Elevación: 0° (horizonte) a 90° (cenit) -> Servo vertical de 20° a 160°
  int posV = map(elev, 0, 90, 20, 160);

  // Filtros de seguridad física
  posH = constrain(posH, 20, 160);
  posV = constrain(posV, 20, 160);

  // Si es de noche (elevación negativa), obligamos al sistema a mirar al Norte quieto
  if (elev < 0) {
    posH = 90; 
    posV = 20; 
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
  
  // -------- MONITOR SERIAL (PC) --------
  Serial.print("Hora: ");
  if(now.hour() < 10) Serial.print('0');
  Serial.print(now.hour()); Serial.print(':');
  if(now.minute() < 10) Serial.print('0');
  Serial.print(now.minute());

  Serial.print(" | Luz (W/m2): "); Serial.print(irradiancia);
  Serial.print(" | Azim Real: "); Serial.print(azim);
  Serial.print(" | Elev Real: "); Serial.println(elev);

  // -------- ENVIAR AL ESCLAVO --------
  esclavo.print(posH);
  esclavo.print(",");
  esclavo.println(posV);

  delay(2000); 
}
