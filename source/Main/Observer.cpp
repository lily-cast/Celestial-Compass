#include "Observer.h"
#include "CONFIG.h"

float RADECtable[3][2] = {
  {214.1933333,19.05358333},
  {37.95454167,89.26391111},
  {160,56}
};

String nameTable[3] = {
  "Arcturus",
  "Polaris",
  "Ursa Major"
};

Observer::Observer() {
  PlanetCalc.begin();
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
  this->currentDay = 27;
  this->currentTime = 20.5; // just pretend it's noon UTC

  PlanetCalc.setGMTdate(currentYear, currentMonth, currentDay);
  PlanetCalc.setGMTtime(20, 30, 0);
  PlanetCalc.setLatLong(latitude, longitude);
  PlanetCalc.getGMT();
}

Horizon Observer::calculatePosition(int objectID) {
  // TODO: create lookup table for object IDs and their RA and DEC
  Serial.println();
  Serial.print("Calculating position: ");
  String name = objectName(objectID);
  Serial.println(name);
  struct RADEC coordinates = objectLookup(objectID);
  return AzimuthAltitude(coordinates);
}

Horizon Observer::AzimuthAltitude(RADEC coords) {
  double rightAscension = coords.RightAscension;
  double declination = coords.Declination;

  // grab the sidereal time
  float LST = siderealTime();
  //Serial.println("LST: " + String(LST));

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

Cartesian Observer::KeplerianHeliocentric(double kepElms[]) {
  // this currently doesn't work! so we'll actually use a different method to calculate the planets and moon

  // the keplerian elements required to calculate position are stored in the array as follows
  // a0 :: semi-major axis (au)
  // e0 :: eccentricity (unitless)
  // I0 :: inclination (degrees)
  // L0 :: mean longitude (degrees)
  // w0 :: longitude of perihelion (degrees)
  // o0 :: longitude of ascending node (degrees)

  // after that are all the time rate versions of the previous variables
  // aDot :: rate of change of semi-major axis (au/century)
  // eDot :: rate of change of eccentricity (unitless)
  // IDot :: rate of change of inclination (degrees/century)
  // LDot :: rate of change of mean longitude (degrees/century)
  // wDot :: rate of change of longitude of perihelion (degrees/century)
  // oDot :: rate of change of longitdue of ascending node (degrees/century)

  // for Jupiter, Saturn, Uranus, and Neptune they all get extra terms to help compute their location
  // these are 0 for everyone else

  // not sure what they stand for but this is the order they come in
  // b
  // c
  // s
  // f

  // lets store these locally so the equations are a little more legible
  double a0 = kepElms[0];
  double e0 = kepElms[1];
  double I0 = kepElms[2];
  double L0 = kepElms[3];
  double w0 = kepElms[4];
  double o0 = kepElms[5];

  double aDot = kepElms[6];
  double eDot = kepElms[7];
  double IDot = kepElms[8];
  double LDot = kepElms[9];
  double wDot = kepElms[10];
  double oDot = kepElms[11];

  double b = kepElms[12];
  double c = kepElms[13];
  double s = kepElms[14];
  double f = kepElms[15];

  // first, begin by fetching the julianDate
  double T = julianDate();
  // we'll be tricky and actually use the number of centuries since J2000, computed as follows
  double D_tt = (T + (currentTime/(float)24)) - 2451545;

  T = D_tt / (float)36525;
  // using the julian dates, calculate the current value of our keplerian elements
  double a = a0 + aDot*T;
  double e = e0 + eDot*T;
  double I = I0 + IDot*T;
  double L = L0 + LDot*T;
  double w = w0 + wDot*T;
  double o = o0 + oDot*T;

  while(L < 0){L += 360;}
  while(L > 360){L -=360;}
  

  double e_star = e * DEG_over_RAD;
  // nice, now let's go ahead and comput the argument of perihelion (AOPh)
  double AOPh = w - o;

  while(AOPh < 0){AOPh += 360;}
  while(AOPh > 360){AOPh -=360;}

  // also compute the mean anomaly (M)
  double M = L - w + b*(T*T) + c*cosd(f*T)+ s*sind(f*T);

  // ensure that M is within 180 degrees of 0
  while(M < -180) {
    M += 360;
  }
  while(M > 180) {
    M -= 360;
  }

  // with that out of the way lets find the eccentric anomaly
  // we'll need to find a solution to keplers equation

  // finally calculate the Eccentric Anomaly
  //double EccAnom = (float(1)/(1-e))*M + (e/pow((1-e),4))*(pow(M,3)/(6)) + ((9*pow(e,2) + e)/pow((1-e),7))*(pow(M,5)/(120));

   // keep track of how many cycles we've run. Tune this to improve performance
  int cycles = 0;
  int maxCycles = 125;

  // also track the error
  float err = 1;
  float errTol = 0.000001;

  // and our current and previous guess
  double EccAnom = M + e_star*sind(M);
  double lastEccAnom;
  
  // finally calculate the Eccentric Anomaly
  while(cycles < maxCycles) {
    // assign swap our EccAnom predictions
    lastEccAnom = EccAnom;

    double dM = M - (EccAnom - (e_star*sind(EccAnom)));
    double dEccAnom = dM / (1 - (e*cosd(EccAnom)));

    EccAnom += dEccAnom;

    if(cycles != 0) {
      err = abs(EccAnom - lastEccAnom);
      if (err <= errTol) {
        cycles = maxCycles; // brute force way to end the while loop
      }
    }
    cycles++; 
  }

  // using that, we can compute the planet's heliocentric coordinates in its orbital plane
  double x_prime = a * (cosd(EccAnom) - e);
  double y_prime = a*sqrt(float(1) - (e*e)) * sind(EccAnom);
  double z_prime = float(0);

  // now calculate the coordinates in the j2000 ecliptic plane
  double x_ecl = ((cosd(AOPh)*cosd(o) - sind(AOPh)*sind(o)*cosd(I)) *x_prime) +
                ((-sind(AOPh)*cosd(o) - cosd(AOPh)*sind(o)*cosd(I)) *y_prime);

  double y_ecl = ((cosd(AOPh)*sind(o) + sind(AOPh)*cosd(o)*cosd(I)) *x_prime) +
                ((-sind(AOPh)*sind(o) + cosd(AOPh)*cosd(o)*cosd(I)) *y_prime);

  double z_ecl = ((sind(AOPh)*sind(I)) *x_prime) + ((cosd(AOPh)*sind(I)) *y_prime);

  // now we can obtain the equatorial coordinates in the J2000 frame using the obliquity
  double obliq = 23.43928;

  double x_eq = x_ecl;
  double y_eq = cosd(obliq)*y_ecl - sind(obliq)*z_ecl;
  double z_eq = sind(obliq)*y_ecl - cosd(obliq)*z_ecl;

  // store those variables and send them out!
  struct Cartesian heliocentricCoords;
  heliocentricCoords.x = x_eq;
  heliocentricCoords.y = y_eq;
  heliocentricCoords.z = z_eq;
 
  // Debug stuff
  Serial.println("------ Planetary Coordinates ------");
  Serial.println("x: " + String(x_eq));
  Serial.println("y: " + String(y_eq));
  Serial.println("z: " + String(z_eq));

  Serial.println("Mean anomaly: " + String(M));
  Serial.println("Eccentric anomaly: " + String(EccAnom));
  Serial.println();
  Serial.println("SMA: " + String(a));
  Serial.println("Ecc: " + String(e));
  Serial.println("Inc: " + String(I));
  Serial.println("Lon: " + String(L));
  Serial.println("LPi: " + String(w));
  Serial.println("LAN: " + String(o));

  Serial.println("Argument of Perihelion: " + String(AOPh));

  Serial.println("----------------------------------");

  return heliocentricCoords;
}

float Observer::siderealTime() {
  // this is all an approximation, but it does the job

  // first calculate the julian date, courtesy of the SolarPosition library
  long currentJD = julianDate();

  // TESTING //
  //Serial.println("Current JD: " + String(currentJD));

  //float LST = 100.4606184 + (0.985647 * currentJD) + longitude + (15*currentTime);

  double D_ut = currentJD - 2451545;
  double D_tt = (currentJD + (currentTime/(float)24)) - 2451545;

  double T = D_tt / (float)36525;

  double GMST = (6.697375 +  0.065707485828*D_ut + 1.0027379*currentTime + 0.0000258*(T*T));
  // fake modulo cause silly arduino doesn't want me to use modulo on anything but ints
  float GMST_decimals = GMST - floor(GMST); // extract the decimals from the number

  GMST = ((int)floor(GMST) % 24) + GMST_decimals;

  //Serial.println("GMST: " + String(GMST));
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

long Observer::julianDate() {
  int A,B;
  int localMonth = currentMonth;
  int localYear = currentYear;
  if(currentMonth <= 2) {
    localYear--;
    localMonth += 12;
  }
  A = localYear / 100;
  B = 2 - A + A/4;
  long currentJD = (365.25 * (localYear + 4716)) + (30.6001 * (localMonth + 1)) +  currentDay + B - 1524;
  return currentJD;
}

RADEC Observer::Car_to_RADEC(Cartesian Coords) {
  // from here, we can finally convert to a right ascension and declination
  double x = Coords.x;
  double y = Coords.y;
  double z = Coords.z;

  double r = sqrt((x*x) + (y*y) + (z*z)); // magnitude of the vector

  double dec = asin(z / r) * DEG_over_RAD; // yay!
  double ra = asin(y/ (r*cosd(dec)) ) * DEG_over_RAD;

  struct RADEC outputCoords;
  outputCoords.Declination = dec;
  outputCoords.RightAscension = ra;
  return outputCoords;
}

float Observer::cosd(float deg) {
  return cos(deg / DEG_over_RAD);
}

float Observer::sind(float deg) {
  return sin(deg / DEG_over_RAD);
}

RADEC Observer::objectLookup(int objectID) {
  struct RADEC objectCoords;
  if (objectID >= 1000 && objectID < 2000) {
    switch(objectID) {
      case 1000:
        PlanetCalc.doSun();
        break;
      /*case 1001:
        PlanetCalc.doMercury();
        break;
      case 1002:
        PlanetCalc.doVenus();
        break;*/
      case 1003:
        PlanetCalc.doMoon();
        break;
      /*case 1004:
        PlanetCalc.doMars();
        break;
      case 1005:
        PlanetCalc.doJupiter();
        break;
      case 1006:
        PlanetCalc.doSaturn();
        break;
      case 1007:
        PlanetCalc.doUranus();
        break;
      case 1008:
        PlanetCalc.doNeptune();
        break;*/
    }
    objectCoords.RightAscension = PlanetCalc.getRAdec() * 15.0;
    objectCoords.Declination = PlanetCalc.getDeclinationDec();
  } else {
    objectCoords.RightAscension = RADECtable[objectID-2000][0];
    objectCoords.Declination = RADECtable[objectID-2000][1];
  }

  return objectCoords;
}

String Observer::objectName(int objectID) {
  if(objectID >= 1000 && objectID < 2000) {
    return "planet";
  } else {
    return nameTable[(objectID-2000)];
  }
}