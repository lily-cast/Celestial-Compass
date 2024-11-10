#include "Observer.h"

Observer::Observer() {

  init();
}

void Observer::init() {
  // initalizes the observer

  // for now, assume this is at my house! Hello anyone reading this who now knows where I live. Please be cool about it
  this->longitude = -122.1038;
  this->latitude = 47.72195;

  // likewise, manually set this for now
  this->currentYear = 2024;
  this->currentMonth = 11;
  this->currentDay = 10;
  this->currentTime = 22; // just pretend it's noon UTC
}

Horizon Observer::calculatePosition(int objectID) {
  // TODO: create lookup table for object IDs and their RA and DEC
  float rightAscension;
  float declination;
  switch(objectID) {
    case 1:
      // for now this will be Polaris
      rightAscension = 15 * (2 + (31/60) + (49.09/3600));
      declination = (89 + (15/60) + (50.08/3600));
      break;
    case 2:
      // assume this is ursa major
      rightAscension = 15 * (10 + (40/60) + (0/3600));
      declination = (56 + (0/60) + (0/3600));
      break;
    case 3:
      // arcturus, part of the bootes constellation
      rightAscension = 214.19333333333;
      declination = 19.053583333333;
      break;
  }
  // grab the sidereal time
  float LST = siderealTime();
  Serial.println("LST: " + String(LST));

  // convert RA to Hour Angle
  float HA = LST*15 - rightAscension;

  // constrain to 0-360
  while(HA < 0) {HA += 360;}
  while(HA >= 360) {HA -=360;}

  // before moving forward, convert everything to radians
  HA = (PI/(float)180) * HA;
  float dec = (PI/(float)180) * declination;
  float lat = (PI/(float)180) * latitude;

  // convert to azimuth and altitude
  float azimuth = atan2((sin(HA)),(cos(HA)*sin(lat) - tan(dec)*cos(lat))) - PI;
  float altitude = asin(sin(lat)*sin(dec) + cos(lat)*cos(dec)*cos(HA));
  
  // convert back to degrees
  azimuth = (180/PI) * azimuth;
  altitude = (180/PI) * altitude;
  // keep that baby positive
  if(azimuth < 0) {azimuth += 360;}

  struct Horizon coordinates;
  coordinates.azimuth = azimuth;
  coordinates.altitude = altitude;

  Serial.println("Local azimuth: " + String(azimuth));
  Serial.println("Local altitude: " + String(altitude));
  return coordinates;
}

float Observer::siderealTime() {
  // this is all an approximation, but it does the job
  
  // first calculate the julian date, courtesy of the SolarPosition library
  long currentJD;
  int A,B;
  int localMonth = currentMonth;
  int localYear = currentYear;
  if(currentMonth <= 2) {
    localYear--;
    localMonth += 12;
  }
  A = localYear / 100;
  B = 2 - A + A/4;
  currentJD = (365.25 * (localYear + 4716)) + (30.6001 * (localMonth + 1)) +  currentDay + B - 1524;

  // TESTING // 
  Serial.println("Current JD: " + String(currentJD));

  //float LST = 100.4606184 + (0.985647 * currentJD) + longitude + (15*currentTime);

  double D_ut = currentJD - 2451545;
  double D_tt = (currentJD + (currentTime/(float)24)) - 2451545;

  double T = D_tt / (float)36525;

  double GMST = (6.697375 +  0.065707485828*D_ut + 1.0027379*currentTime + 0.0000258*(T*T));
  // fake modulo cause silly arduino doesn't want me to use modulo on anything but ints
  float GMST_decimals = GMST - floor(GMST); // extract the decimals from the number
  
  GMST = ((int)floor(GMST) % 24) + GMST_decimals;

  Serial.println("GMST: " + String(GMST));
  float LST = GMST + longitude/(float)15;

  // keep this within 360
  while(LST < 0 ) {
    LST += 360;
  }
  while(LST >= 360) {
    LST -= 360;
  }
  return LST;
}