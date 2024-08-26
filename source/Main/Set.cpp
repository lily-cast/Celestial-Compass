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
  this->updateFreq = 5 * 1000; // update the object loccation once every 5 seconds
  this->maxSpeed = 50; // max speed in deg/sec
  this->targetAzi = 0;
  this->targetAlt = 0;
  this->lastUpdate = millis();

  this->objectID = 0; // this will get fleshed out more, but for now let's assume that 0 is polaris
}

void Set::update() {
  // check to see if we need to update the location of the tracked object
  if(millis() >= lastUpdate + updateFreq) {
    // check the location of the object
    calculateAzAlt();
  }
  // finally, update the motors
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
  // now, subtract and then make sure the angle is within bounds
  float deltaAngle = targetAzi - currentP;
  while(deltaAngle < 0) {
    deltaAngle += 360;
  }
  while(deltaAngle>=360) {
    deltaAngle -= 360;
  }
  // now, check to see what side we should be on
  bool directionP = (deltaAngle <= 180);
  // set the direction of the P motor
  mP.setDirection(directionP);

  // now, we need to know if the R motor should move together or with
  // the P gr_motor_P
  // only move in opposite directions if final positions are splitting away from both poitsn
  float currentR = mR.getAngle()/gr_motor_R;
  float deltaP = Az - currentP;
  if(deltaP < -180) {
    deltaP += 360;
  } else if(deltaP >= 180) {
    deltaP -= 360;
  }

  float deltaR = (Az + extraAngle) - currentR;
  if(deltaR < -180) {
    deltaP += 360;
  } else if(deltaR >= 180) {
    deltaR -= 360;
  }
  
  Serial.println("DeltaP: " + String(deltaP));
  Serial.println("DeltaR: " + String(deltaR));
  // test to see if we would optimally go opposite directions
  if(deltaR*deltaP < 0) {
    Serial.println("Might need to flip directions...");
    // now make sure we don't have any intersections
    if(deltaP > 0 && Az < currentR) {
      mR.setDirection(!directionP);
      Serial.println("Flipping directions");
    } else if(deltaP < 0 && currentP < (Az + extraAngle)) {
      mR.setDirection(!directionP);
      Serial.println("Flipping directions");
    } else {
      mR.setDirection(directionP);
      Serial.println("keeping them the same!");
    }
  } else {
    mR.setDirection(directionP);
  }
}

bool Set::checkMotors() {
  return (mP.checkAngle() && mR.checkAngle());
}

void Set::setStepResolution(int res) {
  mP.setStepResolution(res);
  mR.setStepResolution(res);
}

void Set::calculateAzAlt() {
  // get the current time
  // TEST //
}