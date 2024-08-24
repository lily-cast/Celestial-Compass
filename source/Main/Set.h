// Short for 'gear set' and is the class that actually controls the
// motion of the gears. Works with the motor class to combine their motion
// (and some information about gear ratios) to create smooth motion to keep
// the motion of the entire set smooth.line

#ifndef SET_H
#define SET_H

#include <Arduino.h>
#include "Motor.h"

class Set {
  private:
    // store information about the gear ratios
    float gr_motor_P; // ratio of motor diameter to P gear diameter
    float gr_motor_R; // ratio of motor diameter to R gear diameter
    float gr_R_Alt; // ratio of R gear diameter to altitude control diameter
    float maxSpeed; // the maximum speed the fastest gear will spin (usually the R gear)

    Motor mP; // motor object for the P gear
    Motor mR; // motor object for the R gear

    // information about the object tracking
    int objectID; // ID of the object current being tracked
    float updateFreq; // rate at which the location of the tracked object is updated
    float currentAzi; // current azimuth
    float cuurentAlt; // current altitude
    float targetAzi; // desired azimuth to point to (deg), where 0 deg is directly north
    float targetAlt; // desired altitude (deg), where 0 deg is perfectly level with the horizon
  public:
    Set(int pinsP[], int pinsR[], float gr_motor_P, float gr_motor_R, float gr_R_Alt);
    void init();
    void update(); // updates the position of the motors and tracking of object
    void trackObject(int ID); // tells the Set which object ID to track
    void setAzAlt(float Az, float Alt); // forcibly sets the azimuth and alititude, will most likely be removed later
};

#endif