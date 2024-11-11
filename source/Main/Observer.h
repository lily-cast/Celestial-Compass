// this class handles the actual observation of different celestial bodies.
// It is meant to be created at the start of the program and fed information from
// both the device and the internet. However, it will not keep track of specific
// objects, and instead the observer will be passed along to each Set, for it to
// call on specific object IDs to get their azimuth and altitudes

#ifndef OBSERVER_H
#define OBSERVER_H

#include <Arduino.h>

// define a struct to store and send azimuth and altitude coordinates
struct Horizon {
  float azimuth;
  float altitude;
};

struct RADEC {
  float RightAscension;
  float Declination;
};

class Observer {
  private:
    // current position of the observer
    float longitude; // degs
    float latitude; // degs

    // timekeeping variables, MUST BE IN UTC
    float currentTime; // kept as hours, decimals for minutes and seconds and etc.
    int currentDay; // 1-31
    int currentMonth; // 1-12
    int currentYear;

  public:
    Observer(); // constructor for the observer
    void init(); // initializes the observer

    float cosd(float deg); // literally just a degrees based cosine function
    float sind(float deg); // same here

    Horizon calculatePosition(int objectID); // calculates the azimuth and altitude of a given object

    // functions for celestial math
    long julianDate(); // 
    float siderealTime(); // calculates sidereal time from the current date and time
    RADEC KeplerianObject(float kepElms[]); // finds the RA and DEC of an object given the required keplerian elements

};
#endif