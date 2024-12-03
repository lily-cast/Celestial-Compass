// Short for 'gear set' and is the class that actually controls the
// motion of the gears. Works with the motor class to combine their motion
// (and some information about gear ratios) to create smooth motion to keep
// the motion of the entire set smooth.line

#ifndef SET_H
#define SET_H

#include <Arduino.h>
#include "Motor.h"
#include "Observer.h"

class Set {
  private:
    // store information about the gear ratios
    float gr_R_Alt; // ratio of R gear diameter to altitude control diameter
    float maxSpeed; // the maximum speed the fastest gear will spin (usually the R gear)

    Motor mP; // motor object for the P gear
    Motor mR; // motor object for the R gear

    // information about the object tracking

    float updateFreq; // rate at which the location of the tracked object is updated
    float currentAzi; // current azimuth
    float cuurentAlt; // current altitude

    float currentP; // current angular position of P motor
    float currentR; // current angular position of R motor
    
    float targetAzi; // desired azimuth to point to (deg), where 0 deg is directly north
    float targetAlt; // desired altitude (deg), where 0 deg is perfectly level with the horizon

    // timing variables
    unsigned long lastUpdate; // time since the last location update

    // object selection variables and things
    int pin_left;
    int pin_right;
    bool left_prevState;
    bool right_prevState;
    float lastPress; // time in ms since the last press, helpful to prevent bouncy buttons

    void checkSelection(); // checks the object selection buttons to see if we need to change the object selection from our bank
  public:
    Set(int pinsP[], int pinsR[], int pin_left, int pin_right, float gr_motor_P, float gr_motor_R, float gr_R_Alt);
    void init();
    void update(); // updates the position of the motors and tracking of object
    void trackObject(int ID); // tells the Set which object ID to track
    void setAzAlt(Horizon AzAlt); // forcibly sets the azimuth and alititude, will most likely be removed later
    bool checkMotors(); // checks if motors are in place
    void setStepResolution(int res); // sets both motors to the desired resolution

    void Set::calculateAzAlt(); // calculates azimuth and altitude
    void Set::setObserver(Observer newObserver);

    void setObjectBank(int newBank[]);


    int objectID; // ID of the object current being tracked

    Observer* objectObserver; // keeps track of objects
};

#endif