#include <SoftwareSerial.h>
#include "RTClib.h"
#include <math.h>

//Dejando todo como antes
SoftwareSerial puertoEsclavo(2, 3); 

RTC_DS1307 rtc;
int sensor = A0;
float lat = 6.24 * PI / 180; //Latitud de Medellín en radianes

//Función para mapear con precisión decimal
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) 
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() 
{
  Serial.begin(9600);        // Abre el Monitor Serie de Arduino
  puertoEsclavo.begin(9600); // Abre la comunicación hacia el Esclavo
  
  if (!rtc.begin()) 
  {
    Serial.println("Error: No se encuentra el módulo RTC");
  }

  Serial.println("Hora,Irradiancia(W/m2),Angulo_Horizontal,Angulo_Vertical");
}

void loop() {
  //LECTURA DEL TIEMPO (RTC)
  DateTime now = rtc.now();
  int diasMes[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int n = diasMes[now.month() - 1] + now.day();
  float hourDecimal = now.hour() + now.minute() / 60.0 + now.second() / 3600.0;

  //CÁLCULO ASTRONÓMICO SOLAR
  float delta = 23.45 * sin((360.0 / 365.0) * (n - 81) * PI / 180) * PI / 180;
  float H = 15 * (hourDecimal - 12) * PI / 180;

  float sin_alpha = sin(lat) * sin(delta) + cos(lat) * cos(delta) * cos(H);
  float alpha = asin(sin_alpha);
  float cosA = (sin(delta) - sin(alpha) * sin(lat)) / (cos(alpha) * cos(lat));
  float A = acos(constrain(cosA, -1.0, 1.0)); 

  float elev = alpha * 180 / PI;
  float azim = A * 180 / PI;

  if (H > 0) {
    azim = 360.0 - azim;
  }

  //TRADUCCIÓN A ÁNGULOS DE SERVO
  float posH;
  if (azim <= 180) {
    posH = mapFloat(azim, 0.0, 180.0, 90.0, 160.0); 
  } else {
    posH = mapFloat(azim, 180.0, 360.0, 20.0, 90.0);  
  }
  float posV = mapFloat(elev, 0.0, 90.0, 20.0, 160.0);

  // Límites de seguridad física
  posH = constrain(posH, 20.0, 160.0);
  posV = constrain(posV, 20.0, 160.0);
  
  if (elev < 0) { // Modo nocturno (retorno al origen)
    posH = 90.0; 
    posV = 20.0; 
  }

  // -------- 4. CÁLCULO DE IRRADIANCIA (FOTODIODO) --------
  const float RL = 100000.0;        
  const float RESPONSIVIDAD = 0.62; 
  const float AREA_M2 = 1.37e-5;    
  int lectura = analogRead(sensor); 
  float voltaje = lectura * (5.0 / 1023.0);
  float corriente = voltaje / RL;
  float potenciaW = corriente / RESPONSIVIDAD;
  float irradiancia = potenciaW / AREA_M2;
  
  // -------- 5. SALIDA AL MONITOR SERIE (Para copiar a Excel) --------
  if(now.hour() < 10) Serial.print('0');
  Serial.print(now.hour()); Serial.print(':');
  if(now.minute() < 10) Serial.print('0');
  Serial.print(now.minute()); Serial.print(':');
  if(now.second() < 10) Serial.print('0');
  Serial.print(now.second());
  
  Serial.print(","); Serial.print(irradiancia);
  Serial.print(","); Serial.print(posH, 2);
  Serial.print(","); Serial.println(posV, 2);

  // -------- 6. ENVÍO DE DATOS EN CAPSULADOS AL ESCLAVO --------
  puertoEsclavo.print("<");
  puertoEsclavo.print(posH, 2);
  puertoEsclavo.print(",");
  puertoEsclavo.print(posV, 2);
  puertoEsclavo.println(">");

  delay(2000); // Mide y actualiza cada 2 segundos
}
