#include <SoftwareSerial.h>
#include <Servo.h>

// Comunicación con el Arduino Maestro (RX, TX)
SoftwareSerial maestro(2, 3);

Servo servoH;
Servo servoV;

String data = "";

void setup() {
  Serial.begin(9600);
  maestro.begin(9600);

  servoH.attach(9);
  servoV.attach(10);

  servoH.write(90);
  servoV.write(90);
}

void loop() {
  while (maestro.available()) {
    char c = maestro.read();


    if (c == '\n') {
      int commaIndex = data.indexOf(',');

      int posH = data.substring(0, commaIndex).toInt();
      int posV = data.substring(commaIndex + 1).toInt();

      posH = constrain(posH, 20, 160);
      posV = constrain(posV, 20, 160);

      servoH.write(posH);
      servoV.write(posV);

      Serial.print("Servo H: ");
      Serial.print(posH);
      Serial.print("  Servo V: ");
      Serial.println(posV);

      data = "";
    } else {
      data += c;
    }
  }
}
