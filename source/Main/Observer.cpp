#include "Observer.h"
#include "CONFIG.h"

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
  this->currentDay = 12;
  this->currentTime = 1.25; // just pretend it's noon UTC
}

Horizon Observer::calculatePosition(int objectID) {
  // TODO: create lookup table for object IDs and their RA and DEC
  float rightAscension;
  float declination;

  if(objectID == 10) {
    // find mars!
    double mars[16] = {1.52371034, 0.09339410, 1.84969142, -4.55343205, -23.94362959, 49.55953891, 
                      0.00001847, 0.00007882, -0.00813131, 19140.30268499, 0.44441088, -0.29257343, 
                      0, 0, 0, 0};
    struct RADEC coords = KeplerianObject(mars);
    rightAscension = coords.RightAscension;
    declination = coords.Declination;
  } else {


    switch(objectID) {
      case 0:
        // for now this will be Polaris
        rightAscension = 15 * (2 + (31/60) + (49.09/3600));
        declination = (89 + (15/60) + (50.08/3600));
        break;
      case 1:
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

RADEC Observer::KeplerianObject(double kepElms[]) {
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

  Serial.println(T, 20);
  // using the julian dates, calculate the current value of our keplerian elements
  double a = a0 + aDot*T;
  double e = e0 + eDot*T;
  double I = I0 + IDot*T;
  double L = L0 + LDot*T;
  double w = w0 + wDot*T;
  double o = o0 + oDot*T;

  double e_star = e * DEG_over_RAD;
  // nice, now let's go ahead and comput the argument of perihelion (AOPh)
  double AOPh = w - o;

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

  // from here, we can finally convert to a right ascension and declination
  double r_eq = sqrt((x_eq*x_eq) + (y_eq*y_eq) + (z_eq*z_eq)); // magnitude of the vector

  double dec = asin(z_eq / r_eq) * DEG_over_RAD; // yay!
  double ra = asin(y_eq/ (r_eq*cosd(dec)) ) * DEG_over_RAD;

  struct RADEC coords;
  coords.Declination = dec;
  coords.RightAscension = ra;

  Serial.println("Right ascension: " + String(ra));
  Serial.println("Declination: " + String(dec));

  Serial.println("x: " + String(x_ecl));
  Serial.println("y: " + String(y_ecl));
  Serial.println("z: " + String(z_ecl));

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
 
  return coords;
}

float Observer::siderealTime() {
  // this is all an approximation, but it does the job

  // first calculate the julian date, courtesy of the SolarPosition library
  long currentJD = julianDate();

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

float Observer::cosd(float deg) {
  return cos(deg / DEG_over_RAD);
}

float Observer::sind(float deg) {
  return sin(deg / DEG_over_RAD);
}