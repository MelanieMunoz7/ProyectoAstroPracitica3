//Conectaremos los arduinos con Wire, conexion 12C
#include <Wire.h>
#include "RTClib.h"
#include <math.h>

RTC_DS1307 rtc;

int sensor = A0;
float lat = 6.24 * PI / 180; //Latitud Medellín en radianes
const int DIRECCION_ESCLAVO = 8;

//Estructura union para descomponer el float en 4 bytes, porque Wire solo manda un byte
union FloatALongitud 
{
  float valorFloat;
  byte bytes[4];
};

//Función para mapear usando flotantes (para no perder precisión)
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) 
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() 
{
  Serial.begin(9600);   
  Wire.begin(); 
  
  if (!rtc.begin()) 
  {
    Serial.println("No se encuentra el RTC");
  }
}

void loop() 
{
  //LEER HORA
  DateTime now = rtc.now();
  int diasMes[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int n = diasMes[now.month() - 1] + now.day();
  float hourDecimal = now.hour() + now.minute() / 60.0 + now.second() / 3600.0;

  //COORDENADAS AZIMUTALES
  float delta = 23.45 * sin((360.0 / 365.0) * (n - 81) * PI / 180) * PI / 180;
  float H = 15 * (hourDecimal - 12) * PI / 180;

  float sin_alpha = sin(lat) * sin(delta) + cos(lat) * cos(delta) * cos(H);
  float alpha = asin(sin_alpha);
  float cosA = (sin(delta) - sin(alpha) * sin(lat)) / (cos(alpha) * cos(lat));
  float A = acos(constrain(cosA, -1.0, 1.0)); 

  float elev = alpha * 180 / PI;
  float azim = A * 180 / PI;

  if (H > 0) 
  {
    azim = 360.0 - azim;
  }

  //MAPEO A SERVOS CON DECIMALES (Flotantes)
  float posH;
  if (azim <= 180) 
  {
    posH = mapFloat(azim, 0.0, 180.0, 90.0, 160.0); 
  } 
  else 
  {
    posH = mapFloat(azim, 180.0, 360.0, 20.0, 90.0);  
  }

  float posV = mapFloat(elev, 0.0, 90.0, 20.0, 160.0);

  //Filtros de seguridad en formato float (Esto lo sugirió Gemini)
  posH = constrain(posH, 20.0, 160.0);
  posV = constrain(posV, 20.0, 160.0);

  if (elev < 0) 
  {
    posH = 90.0; 
    posV = 20.0; 
  }

  //CÁLCULO DE IRRADIANCIA 
  const float RL = 100000.0;        
  const float RESPONSIVIDAD = 0.62; 
  const float AREA_M2 = 1.37e-5;    
  int lectura = analogRead(sensor); 
  float voltaje = lectura * (5.0 / 1023.0);
  float corriente = voltaje / RL;
  float potenciaW = corriente / RESPONSIVIDAD;
  float irradiancia = potenciaW / AREA_M2;
  
  //MONITOR SERIAL
  Serial.print("Hora: ");
  if(now.hour() < 10) Serial.print('0');
  Serial.print(now.hour()); Serial.print(':');
  if(now.minute() < 10) Serial.print('0');
  Serial.print(now.minute());
  Serial.print(" | Luz (W/m2): "); Serial.print(irradiancia);
  Serial.print(" | TargetH: "); Serial.print(posH, 4); // Muestra 4 decimales
  Serial.print(" | TargetV: "); Serial.println(posV, 4);

  //ENVIAR AL ESCLAVO POR I2C (8 BYTES EN TOTAL)
  FloatALongitud datoH;
  FloatALongitud datoV;
  
  datoH.valorFloat = posH;
  datoV.valorFloat = posV;

  Wire.beginTransmission(DIRECCION_ESCLAVO);
  // Enviamos los 4 bytes del Horizontal
  Wire.write(datoH.bytes, 4);
  // Enviamos los 4 bytes del Vertical
  Wire.write(datoV.bytes, 4);
  Wire.endTransmission();

  delay(2000); 
}
