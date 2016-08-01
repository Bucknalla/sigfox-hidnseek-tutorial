/* detectMovement

 Dependent on movement, transmits the device's GPS location via a Sigfox message.

 Created 2016 by Alex Bucknall

 Referencing Stephane Driussi (2016)
*/

#define txSigfox         5     // PD5 TX Serial to Sigfox modem
// #define bluLEDpin        6     // PD6 Piezzo Output
#define redLEDpin        7     // PD7 Red LED Status
#define rxSigfox         8     // PB0 RX Serial from Sigfox modem
#define ACCEL_MODE       2
#define SENSITIVITY      2

#include "SoftwareSerial.h"
#include "HidnSeek.h"
HidnSeek HidnSeek(txSigfox, rxSigfox);

#include "Wire.h" //i2c Library
#include "MMA8653.h" //Accelerometer
MMA8653 accel;

int ledState = LOW;
int ledColor = redLEDpin;
uint8_t timestamp = -1;

unsigned long previousMillis = 0;
unsigned long start;

void setup() {
  HidnSeek.initGPIO(false);
  HidnSeek.setSupply(true);
  Wire.begin();
  accel.begin(false, ACCEL_MODE);
  start = millis();
}

bool accelStatus() {
  static int8_t x, y, z;

  boolean accelMove = false;
  byte error = accel.update();

  if (error != 0) accelMove = true;
  else {
    accelMove = ((uint8_t)(abs((int8_t)(accel.getX() - x))) > SENSITIVITY) ? true :
                ((uint8_t)(abs((int8_t)(accel.getY() - y))) > SENSITIVITY) ? true :
                ((uint8_t)(abs((int8_t)(accel.getZ() - z))) > SENSITIVITY) ? true : false;
    x = accel.getX();
    y = accel.getY();
    z = accel.getZ();
  }
  return accelMove;
}

void loop()
{
  HidnSeek.checkBattery();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 200) {
    previousMillis = currentMillis;
    if (accelStatus()) {
      timestamp = -1;
      ledColor = redLEDpin;
    }
    else {
      if (timestamp-- == 0) HidnSeek.setSupply(false);
      // ledColor = bluLEDpin;
    }
    digitalWrite(ledColor, HIGH);
    delay(50);
    digitalWrite(ledColor, LOW);
  }
}
