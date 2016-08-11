#include <HidnSeek.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include "definition.h" // Required for GPS Commands, etc.

HidnSeek HidnSeek(txSigfox, rxSigfox);

TinyGPS gps;

void initGPIO() // Required to initialise the battery and other GPIO
{
  // Set output to 0 for not used Pads
  PORTB = (DIGITAL_PULLUP >> 8) & 0xff;
  DDRB  = (DIGITAL_OUTPUT >> 8) & 0xff;
  PORTC = 0x00;
  DDRC  = DDRC_MASK;
  PORTD = DIGITAL_PULLUP & 0xff;
  DDRD  = DIGITAL_OUTPUT & 0xff;
}

void serialString (PGM_P s) { // Used to send commands to the GPS
  char c;
  while ((c = pgm_read_byte(s++)) != 0)
    Serial.print(c);
}

/* GPS Code */

void gpsCmd (PGM_P s) { // Allows sending of commands directly to the GPS
  int XOR = 0;
  char c;
  while ((c = pgm_read_byte(s++)) != 0) {
    Serial.print(c);
    if (c == '*') break;
    if (c != '$') XOR ^= c;
  }
  if (XOR < 0x10) Serial.print("0");
  Serial.println(XOR, HEX);
}

bool gpsInit() { // Checks the state of the serial port and set the GPS to output mode to 1 Hz
  boolean GPSready = false;
  digitalWrite(rstPin, HIGH);
  unsigned long startloop = millis();

  while ((uint16_t) (millis() - startloop) < 5000 ) {
    if (Serial.available() > 0 && Serial.read() == '*') {
      GPSready = true;
      break;
    }
    delay(100);
  }
  if (GPSready) {
    gpsCmd(PSTR(PMTK_SET_NMEA_OUTPUT));
    gpsCmd(PSTR(PMTK_SET_NMEA_UPDATE_1HZ));   // 1 Hz update rate
  } else digitalWrite(rstPin, LOW);
  return GPSready;
}

bool gpsProcess() { // Main function for collating and preparing GPS data
  Serial.println("Generating Payload");
  boolean newGpsData = false;
  boolean newSerialData = false;
  float distance;
  unsigned long start = millis();
  unsigned int waitime = 2000;
  // Parse GPS data for 2 second
  while ((uint16_t) (millis() - start) < waitime)
  {
    if (Serial.available() > 0) {
      newSerialData = true;
      waitime = 100;
      start = millis();
      blueLEDon;
    }
    while (Serial.available())
    {
      char c = Serial.read();
      // New valid NMEA data available
      if (gps.encode(c))
      {
        newGpsData = true;
      }
    }
  }

   // Check if NMEA packet received, wake up GPS otherwise
  if (!newSerialData) gpsInit();

  // 12 octets = 96 bits payload
  // lat: 32, lon: 32, alt: 13 , spd: 7, cap: 2, bat: 7, mode: 3 (0-3 sat view, more is MSG)
  // lat: 32, lon: 32, alt:0-8191m, spd:0-127Km/h, bat:0-100%, mode:0-7, cap: N/E/S/W
  // int is 16 bits, float is 32 bits. All little endian

  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);

  if (newGpsData) { // computeData
    gps.f_get_position(&p.lat, &p.lon, &fix_age);
    if (fix_age == TinyGPS::GPS_INVALID_AGE || fix_age > 5000) fix_age = 1024;
    sat = gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites();
    alt = abs(round(gps.f_altitude()));
    spd = round(gps.f_speed_kmph());

    distance = 1000;
    if (fix_age >> 9) {
      newGpsData = false; // No a real fix detected
      p.lat = previous_lat;
      p.lon = previous_lon;
      serialString(PSTR("recover lat="));
      Serial.print(p.lat, 7);
      serialString(PSTR(", lon="));
      Serial.println(p.lon, 7);
    } else if (abs(p.lat) > 2 && abs(p.lon) > 2) distance = gps.distance_between(p.lat, p.lon, previous_lat, previous_lon);
    if (newGpsData && distance < 5 && syncSat > 20 && forceSport == 0) {
      syncSat = 255;
    }

    if (newGpsData) {
      if (sat < 4 || (abs(p.lat) < 2 && abs(p.lon) < 2)) noSat++;
      else {
        noSat = 0;
        syncSat++; // else syncSat = 0; // increase global variable
      }
      if (sat > 7) syncSat ++;
    }
    else noSat++;
  }
  else noSat++;

  printData(newGpsData); // For debug purpose this use 2Ko of flash program

  blueLEDoff;
  return newSerialData;
}

void gpsStandby() { // Function to place the GPS into standby mode
  GPSactive = false;
  digitalWrite(rstPin, LOW);
}

void print_date() // Collects date & time from the GPS and outputs it to serial
{
  char sz[24];
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
          month, day, year, hour, minute, second);
  Serial.print(sz);
}

void printData(bool complete) { // Collects GPS measurements and outputs the data to serial
  print_date();
  serialString(PSTR("fix="));
  Serial.print(fix_age);
  if (complete) {
    serialString(PSTR(", lat="));
    Serial.print(p.lat, 7);
    serialString(PSTR(", lon="));
    Serial.print(p.lon, 7);
    serialString(PSTR(", alt="));
    Serial.print(alt);
    serialString(PSTR(", cap="));
    Serial.print((gps.course() / 90) % 4);
    serialString(PSTR(", spd="));
    Serial.print(spd);
    serialString(PSTR(", sat="));
    Serial.print(sat);
  }

  if (GPSactive) serialString(PSTR(", GPS "));
  serialString(PSTR("%, noSat="));
  Serial.print(noSat);
  serialString(PSTR(", syncSat="));
  Serial.println(syncSat);

}

void makePayload() { // Creates the Payload required for the Sigfox message
  uint8_t cap;
  if (sat > 3) {
    if (alt > 4096) alt = (uint16_t)(alt / 16) + 3840; // 16m step after 4096m
    if (alt > 8191) alt = 8191;                        // 69632m is the new limit ;)

    if (spd > 127) spd = (uint16_t)(spd / 16) + 94; // 16Km/h step after 127Km/h
    else if (spd > 90) spd = (uint16_t)(spd / 3) + 60; // 3Km/h step after 90Km/h
    if (spd > 126) spd = 127;      // limit is 528Km/h
    cap = (gps.course() / 90) % 4;
  } else cap = (accelPosition < 3) ? accelPosition : 3;

  p.cpx = (uint32_t) alt << 19;
  p.cpx |= (uint32_t) spd << 12; // send in Km/h
  p.cpx |= (uint32_t) cap << 10;  // send N/E/S/W
  p.cpx |= (uint32_t) ( 127 & batteryPercent) << 3; // bat (7bits)
  if (sat > 8) sat = 8;
  p.cpx |= (uint32_t) 3 & (sat / 4); // sat range is 0 to 14
}

void decodePayload() { //
  unsigned int alt_ = p.cpx >> 19;
  unsigned int cap_ = (p.cpx >> 10) & 3;
  unsigned int spd_ = (p.cpx >> 12) & 127;
  unsigned int bat_ = (p.cpx >> 3) & 127;
  unsigned int mod_ = p.cpx & 7;
  print_date();
  serialString(PSTR("msg="));
  Serial.print(MsgCount);
  serialString(PSTR(" lat="));
  Serial.print(p.lat, 7);
  serialString(PSTR(", lon="));
  Serial.print(p.lon, 7);
  serialString(PSTR(", alt="));
  Serial.print(alt_);
  serialString(PSTR(", cap="));
  Serial.print(cap_);
  serialString(PSTR(", spd="));
  Serial.print(spd_);
  serialString(PSTR(", bat="));
  Serial.print(bat_);
  serialString(PSTR(", mode="));
  Serial.println(mod_);
}

/* SIGFOX Code */

bool initSigFox() {
  serialString(PSTR("SigFox: "));
  unsigned long previousMillis = millis();
  while ((uint16_t) (millis() - previousMillis) < 6000) {
    if (HidnSeek.begin() == 3) {
      Serial.print(HidnSeek.getID(), HEX);
      return true;
    }
    else delay(200);
  }
  serialString(PSTR("Fail\r\n"));
  return false;
}

void sendSigFox() {

  if(HidnSeek.isReady()){
    Serial.print("Sigfox is ready...");
    makePayload();

    previous_lat = p.lat;
    previous_lon = p.lon;
    decodePayload();
    if(HidnSeek.send(&p, sizeof(p))){
      redLEDon;
       Serial.println("Successful...");
       delay(500);
       redLEDoff;
    }
    else Serial.println("Message Failed...");
  }
  else Serial.println("Sigfox not ready...");
}

/* Main Project Code */

void setup()
{
  init();
  delay(100);

  initGPIO();

  Serial.begin(9600);

  Serial.print("GPS Example Sketch, TinyGPS Version - ");
  Serial.print(TinyGPS::library_version());
  Serial.println();

  if (initSigFox()) { // Checks if the SIGFOX Module is connected & functioning
      delay(500);
  }

  if (GPSactive = gpsInit()) {
      Serial.println("GPS initialised...");
      gpsCmd(PSTR(PMTK_VERSION));
  }
}

void loop()
{
  bool newdata = false;

  delay(5000);
  if (newdata = gpsProcess())
  {
      makePayload();
      sendSigFox();
  }
  delay(5000);
}
