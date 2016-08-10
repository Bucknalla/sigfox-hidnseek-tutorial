/* detectMovement

 Dependent on movement, transmits the device's GPS location via a Sigfox message.

 Library limits device from transmitting more than every 10 minutes to be compliant with EU legislation.

 Adapted by Alex Bucknall 2016

 Originally by Stephane Driussi 2016
*/

#define txSigfox         5     // PD5 TX Serial to Sigfox Module
#define bluLEDpin        6     // PD6 Piezzo Output
#define redLEDpin        7     // PD7 Red LED Status
#define rxSigfox         8     // PB0 RX Serial from Sigfox Module
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

unsigned long previousMillis = 0;
uint16_t counter = 0;
unsigned long start;

void setup() {
    HidnSeek.initGPIO(false);
    HidnSeek.setSupply(true);
    Wire.begin();
    accel.begin(false, ACCEL_MODE);
    HidnSeek.begin();
    start = millis();
}

bool accelStatus() { // Checks if Accelerometer has detected movement
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

void loop(){ // Transmits the number of times the device has been moved since last Sigfox transmission

    HidnSeek.checkBattery(); // Shuts down power supply down if battery is < 3.7 V

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= 200) {
              previousMillis = currentMillis;

        if (accelStatus()) {
          counter += 1;
          if (HidnSeek.isReady()) { // Checks network limit of Sigfox before transmitting the alert
            HidnSeek.send(&counter, sizeof(counter));
            counter = 0;
            digitalWrite(redLEDpin, HIGH);
            delay(200);
            digitalWrite(redLEDpin, LOW);
            delay(800);
          }
          else {
            digitalWrite(bluLEDpin, HIGH);
            delay(200);
            digitalWrite(bluLEDpin, LOW);
            delay(800);
          }
        }

    }
}
