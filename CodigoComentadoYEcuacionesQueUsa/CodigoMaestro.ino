/*Código Maestro*/

//La librerias respectivamente: para la comunicación con el serial, 
//comunicación con el protocolo 12C (Inter-Integrated Circuit, es decir SDA y SCL),
//control del RTC y el cálculo matemático
#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include <math.h>

//Comunicación con el Arduino Esclavo (RX, TX)
SoftwareSerial esclavo(2, 3);

//Crear el objeto reloj
RTC_DS1307 rtc;

//Pin Fotodiodo
int sensor = A0;

//Latitud de Medellín en radianes
float lat = 6.24 * PI / 180;

void setup() 
{
  Serial.begin(9600);   //Inicia conexion con monitor Serial de la PC
  esclavo.begin(9600);  //Inicia comunicación con el otro Arduino
  Wire.begin(); //Comunicacion con el Serial

  //En caso de no detectar el reloj
  if (!rtc.begin()) 
  {
    Serial.println("No se encuentra el RTC");
  }
  
  //Ajusta la hora si es necesario y cada que se suba el código se reajusta
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() 
{
  /*LEER LA HORA*/
  //Obtiene el año, mes, día, hora, minuto, segundo.
  DateTime now = rtc.now();

  //Suma días acumulados por el mes, se debe corregir si el año es bisiesto
  int diasMes[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

  //Obtener puntualmente el día del año en forma de entero.
  int n = diasMes[now.month() - 1] + now.day();
  
  //Convierte toda una hora, con minutos y segundos, a una hora. 
  float hourDecimal = now.hour() + now.minute() / 60.0 + now.second() / 3600.0;

  /*DECLINACIÓN SOLAR Y LUNAR*/
  float delta = 23.45 * sin((360.0 / 365.0) * (n - 81) * PI / 180);
  //Pasandolo a radianes
  delta *= PI / 180;

  /*Ángulo horario*/  
  float H = 15 * (hourDecimal - 12);
  //Pasandolo a radianes
  H *= PI / 180;

  /*Elevación solar*/
  float sin_alpha = sin(lat) * sin(delta) + cos(lat) * cos(delta) * cos(H);
  float alpha = asin(sin_alpha);

  /*Azimut solar*/
  float cosA = (sin(delta) - sin(alpha) * sin(lat)) / (cos(alpha) * cos(lat));
  float A = acos(constrain(cosA, -1.0, 1.0)); //Constrain para evitar errores matemáticos

  //Todo a grados para llevarlo al servo
  float elev = alpha * 180 / PI;
  float azim = A * 180 / PI;

  /*Mapeo a los servos*/
  //Un servo real trabaja entre 20° y 160°, entonces: 
  //0° -mapea-> 20°
  //90° -mapea -> 160°
  int posV = map(elev, 0, 90, 20, 160);
  int posH = map(azim, 0, 180, 20, 160);

  //Evitando valores peligrososo del servo
  posH = constrain(posH, 20, 160);
  posV = constrain(posV, 20, 160);

  /*Leer el fotodiodo*/
  int light = 1023 - analogRead(sensor);

  /*Imprimir en el serial*/
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

  /*Tirandole los datos al esclavo*/
  //Enviamos los datos en una sola línea para que el esclavo los procese
  esclavo.print(posH);
  esclavo.print(",");
  esclavo.println(posV);

  delay(2000); //Actualiza cada 2 segundos
}
