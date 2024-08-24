#include "CONFIG.h"
#include "Motor.h"
#include "Set.h"

Set::Set(int pinsP[], int pinsR[], float gr_motor_P, float gr_motor_R, float gr_R_Alt)
    : mR(pinsR), mP(pinsP) {
  // needs to be given the pins for each motor to correctly work, start by setting those up
  this->gr_motor_P = gr_motor_P;
  this->gr_motor_R = gr_motor_R;
  this->gr_R_Alt = 3;

  init();
}

void Set::init() {
  // set our default parameters
  this->updateFreq = 1; // update the object loccation once every second
  this->maxSpeed = 50; // max speed in deg/sec
  this->targetAzi = 0;
  this->targetAlt = 0;
}

void Set::update() {
  mR.update();
  mP.update();
}

void Set::setAzAlt(float Az, float Alt) {
  // changes the azimuth and altitude while giving the motors new commands
  this->targetAzi = Az;
  this->targetAlt = Alt;

  mP.setAngle(Az * gr_motor_P); // this will always be exactly the azimuth multiplied by the gear ratio
  // in order to calculate the angle of the R gear, we need to add
  // the desired increase in height (relative to the gear ratios) to
  // the angle that the P gear needs to be at
  float extraAngle = Alt / gr_R_Alt; 
  mR.setAngle((Az+extraAngle) * gr_motor_R);

  // now we set the speeds so they get there at the same time
  // first calculate the time for the R gear to get there
  double moveTime = (Az+extraAngle) / maxSpeed;

  mR.setSpeed(maxSpeed);
  mP.setSpeed(Az/moveTime); // just calculate distance/time to get the speed

  // now let's figure out what direction to go
  // first, get the current location of the P motor. Only base movement off of this one
  float currentP = mP.getAngle()/gr_motor_P;
  // now, make sure 
}