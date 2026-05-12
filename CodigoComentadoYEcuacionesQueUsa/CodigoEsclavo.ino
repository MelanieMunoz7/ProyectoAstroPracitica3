/*Código Esclavo*/

//Liberías, una para comunciación con el serial y la otra para controlar los servos
#include <SoftwareSerial.h>
#include <Servo.h>

//Serial virtual para la conexión con el Maestro
SoftwareSerial maestro(2, 3);

//Variables para los dos servos
Servo servoH;
Servo servoV;

//Variables para suavizado
int targetH = 90;
int targetV = 90;

//Posiciones actuales
float currentH = 90;
float currentV = 90;

//Paso para suaviazar el movimiento del motor
float stepSize = 0.05; 

//Ir guardando el mensaje del maestro
String data = "";

void setup() 
{
  Serial.begin(9600);
  //Comunicación con el maestro
  maestro.begin(9600);

  //Conexión de los servos
  servoH.attach(9);
  servoV.attach(10);
  
  //Los servos empiezan en 90°
  servoH.write(currentH);
  servoV.write(currentV);
}

void loop() 
{
  /* 1. Leer datos del maestro*/

  //Mientras para analizar si llegó info al serial
  while (maestro.available()) 
  {
    //Lee un mensaje recibido
    char c = maestro.read();

    //Verifica que el caracter recibido e sun salto de línea
    if (c == '\n') 
    {
      //busca la posición de la coma dentro del mesnaje
      int commaIndex = data.indexOf(',');
      //Si encontro la coma correctamente
      if (commaIndex != -1) 
      {
        //Toma la parte antes de la coma y la vuelve entero
        targetH = data.substring(0, commaIndex).toInt();
        targetV = data.substring(commaIndex + 1).toInt();
      }
      //Limpia memoria para recibir el siguinete mensaje
      data = "";
    } 
    //Si todavía no llega el salto del línea  
    else 
    {
      data += c;
    }
  }

  /* 2. Lógica de suavizado (Movimiento incremental)*/
  //Movimiento Horizontal
  if (abs(targetH - currentH) > 0.1) 
  {
    //Aumentando poco a poco el ángulo
    if (currentH < targetH) currentH += stepSize;
    else currentH -= stepSize;
    servoH.write((int)currentH);
  }

  //Movimiento Vertical
  if (abs(targetV - currentV) > 0.1) 
  {  
    //Aumentando poco a poco el ángulo
    if (currentV < targetV) currentV += stepSize;
    else currentV -= stepSize;
    servoV.write((int)currentV);
  }

  delay(15); //Pequeña pausa para dar tiempo al servo y controlar la velocidad total
}
