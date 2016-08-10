/*  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  th  e Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.

  HidnSeek by StephaneD 2015 Not for commercial use

  Adapted by Alex Bucknall for SIGFOX HidnSeek Tutorial

*/

#include "EEPROM.h"
#include "LowPower.h"
#include "def.h"

#include "HidnSeek.h"
HidnSeek HidnSeek(txSigfox, rxSigfox); //

#include "TinyGPS.h"
TinyGPS gps;

#include "MMA8653.h"
MMA8653 accel;

#include "Barometer.h"
Barometer bmp180;

void serialString (PGM_P s) {
    char c;
    while ((c = pgm_read_byte(s++)) != 0)
    Serial.print(c);
}

void initGPIO(){
    // Set output to 0 for not used Pads
    PORTB = (DIGITAL_PULLUP >> 8) & 0xff;
    DDRB  = (DIGITAL_OUTPUT >> 8) & 0xff;
    PORTC = 0x00;
    DDRC  = DDRC_MASK;
    PORTD = DIGITAL_PULLUP & 0xff;
    DDRD  = DIGITAL_OUTPUT & 0xff;
} // Initialised GPIO Pins

void flashRed(int num) { // Flash red LED for 'num' intervals of 50ms
  while (num > 0) {
    PORTD |= (1 << redLEDpin);
    delay(50);
    PORTD &= ~(1 << redLEDpin);
    if (--num) delay(50);
  }
}

void NoflashRed() {
  delay(25);
  PORTD &= ~(1 << redLEDpin) & ~(forceSport << bluLEDpin);
  delay(50);
  PORTD |= (1 << redLEDpin) | (forceSport << bluLEDpin);
}

int powerDownLoop(int msgs) { // Function to determine the state that the HidnSeek should enter

    if (batterySense()) shutdownSys(); // Checks if the voltage of the battery is too low;
    else gpsStandby();

    if (syncSat < 255) {
        sendSigFox(msgs); // If not around previous location send new position
    }

    accelStatus(); // Record the current angle
    if (msgs == MSG_POSITION && spd > 5 && noSat == 0) detectMotion = MOTION_MIN_NUMBER << 1; else detectMotion = 0;

    // Loop duration 8s. 75x 10mn, 150x 20mn,
    static uint8_t countNoMotion;
    if (msgs != MSG_NO_MOTION) countNoMotion = 0;

    unsigned int waitLoop;

    if (msgs == MSG_NO_MOTION) { 
        waitLoop = 420 << countNoMotion; // 1h loop
        hour += (1 << countNoMotion);

        if (hour > 23) {
            hour = 0;
            day++;
        }
        if (day > 31) {
            day = 0;
            month++;
        }
        if (month > 12) {
            month = 0;
            year++;
        }

    if (countNoMotion < 3) countNoMotion++;

    }
    else {
        waitLoop = (PERIOD_COUNT >> forceSport) - loopGPS;  // 10mn loop: 6mn sleep + 4mn for GPS
    }

    unsigned int i = 0;

    period_t sleepDuration = SLEEP_8S;

    Serial.flush();

    while (i < waitLoop) { // Loop that determines power state

      LowPower.powerDown(sleepDuration, ADC_OFF, BOD_OFF);
      PORTD |= (1 << redLEDpin) | (forceSport << bluLEDpin);

      if (GPSactive) {
          batterySense();
          if (batteryPercent < 98 && !forceSport) gpsStandby();
      }

      if (accelStatus()) { // device moved
          if (GPSactive) NoflashRed();
          else delay(50);

          detectMotion++;

          if (sleepDuration == SLEEP_4S) {
              sleepDuration = SLEEP_8S;
              i = i >> 1;
          }
          if (msgs == MSG_NO_MOTION) waitLoop = 0; // exit immediatly or stay in 5mn loop
      }

      i++;

      PORTD &= ~(1 << redLEDpin) & ~(1 << bluLEDpin);
  }

  detectMotion = (detectMotion > MOTION_MIN_NUMBER) ? 1 : 0;

  if (msgs == MSG_NO_MOTION && waitLoop == 0) detectMotion = -1; // This mean a motion after a while

  if (detectMotion > 0 && !GPSactive) GPSactive = gpsInit();

  start = millis();

  loopGPS = syncSat = noSat = 0;
  alt = spd = 0;
  p.lat = p.lon = 0;

  return detectMotion; // Dictates the state in which the HidnSeek will enter
}

int main(void) {

    init();
    delay(100);

    initGPIO();

    Serial.begin(9600);

    dumpEEprom();
    initSense();
    batterySense();

    serialString(PSTR(" Battery: "));
    Serial.print(batteryPercent);
    serialString(PSTR("% "));
    Serial.println(batteryValue);
    delay(100);

    if (GPSactive = gpsInit()) { // Checks if the GPS is functioning
        gpsCmd(PSTR(PMTK_VERSION));
        flashRed(1);
    }

    if (accelPresent = initMems()) { // Checks if the Accelerometer is functioning
        delay(500);
        if (accelStatus()) flashRed(2);
    }

    if (baromPresent = bmp180.init()) { // Checks if the Barometer is functioning
        delay(500);
        bmp180Measure(&Temp, &Press);
        flashRed(3);
    }

    bmp180Print();

    if (initSigFox()) { // Checks if the SIGFOX Module is functioning
        delay(500);
        flashRed(4);
    }

    else {
        PORTD |= (1 << bluLEDpin);
        Serial.flush();
        delay(500);
        digitalWrite(shdPin, LOW);
        delay(1000);
    }

    start = millis();

    while (1) { // Main Activity of the HidnSeek

        if ((uint16_t) (millis() - start) >= 4000) {
            blueLEDon;
            delay(100);
            accelStatus();
            blueLEDoff;
            loopGPS++;
            start = millis();
        }

        // if a sentence is received, we can check the checksum, parse it...
        if (detectMotion == 1) { //
            if (gpsProcess()) LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);
        }

        // Let 2mn to acquire GPS position otherwise go to sleep until accelerometer wake up.
        if (((syncSat >= 30 && (spd < 11 || spd > 80)) || syncSat >= 60) && noSat == 0) {
            detectMotion = powerDownLoop(MSG_POSITION);
        }

        if (loopGPS > 30 || noSat > 120) {
            detectMotion = powerDownLoop(MSG_NO_GPS);
        }

        if (detectMotion == 0) detectMotion = powerDownLoop(MSG_NO_MOTION);
        if (detectMotion == -1) detectMotion = powerDownLoop(MSG_MOTION_ALERT); // Alert for Motion detected after the blank time

    }
}
