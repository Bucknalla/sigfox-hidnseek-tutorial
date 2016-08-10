#include <SoftwareSerial.h>
#include <HidnSeek.h>
#include <TinyGPS.h>
#include "definition.h" // Required for GPS Commands, etc.

HidnSeek HidnSeek(txSigfox, rxSigfox);

TinyGPS gps;

void initGPIO()
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


bool gpsProcess() {
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
      redLEDon;
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

  redLEDoff;
  return newSerialData;
}

void gpsStandby() { // Function to place the GPS into standby mode
  GPSactive = false;
  digitalWrite(rstPin, LOW);
}

void print_date() // Collects date information from the GPS
{
  char sz[24];
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
          month, day, year, hour, minute, second);
  Serial.print(sz);
}

void printData(bool complete) {
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

void setup()
{
  init();
  delay(100);

  initGPIO();

  Serial.begin(9600);

  Serial.print("GPS Example Sketch ");
  Serial.print(TinyGPS::library_version());

  if (GPSactive = gpsInit()) {
    gpsCmd(PSTR(PMTK_VERSION));
//    flashRed(1);
  }
}

void loop()
{
  bool newdata = false;

  delay(5000);
  if (newdata = gpsProcess())
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
  }
  delay(5000);
}
