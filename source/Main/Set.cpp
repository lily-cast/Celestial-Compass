#include "CONFIG.h"
#include "Motor.h"
#include "Set.h"
#include "Observer.h"

int objectBank[5] = {0,0,0,0,0};
int bankSelection = 2;

Set::Set(int pinsP[], int pinsR[], int pin_left, int pin_right, 
         float gr_motor_P, float gr_motor_R, float gr_R_Alt)
    : mR(pinsR, gr_motor_R), mP(pinsP, gr_motor_P) {
  // needs to be given the pins for each motor to correctly work, start by setting those up
  this->gr_R_Alt = gr_R_Alt;
  this->pin_left = pin_left;
  this->pin_right = pin_right;
  init();
}

void Set::init() {
  // set our default parameters
  this->updateFreq = 5 * 1000; // update the object loccation once every 5 seconds
  this->maxSpeed = 1; // max speed in deg/sec
  this->targetAzi = 0;
  this->targetAlt = 0;
  this->lastUpdate = millis();

  this->objectID = 0; // signifies the set is in manual mode

  // buttons!
  pinMode(pin_left, INPUT_PULLUP);
  pinMode(pin_right, INPUT_PULLUP);
  this->lastPress = millis();
  left_prevState = HIGH;
  right_prevState = HIGH;
}

void Set::update() {
  checkSelection();

  // check to see if we need to update the location of the tracked object
  if(millis() >= lastUpdate + updateFreq) {
    // check the location of the object
    calculateAzAlt();
    lastUpdate = millis();
  }
  // finally, update the motors
  mR.update();
  mP.update();
}

void Set::setAzAlt(Horizon AzAlt) {
  // changes the azimuth and altitude while giving the motors new commands
  float Az = AzAlt.azimuth;
  float Alt = AzAlt.altitude;

  Alt = max(0, Alt);

  this->targetAzi = Az;
  this->targetAlt = Alt;

  

  // first, get the current location of the P and R motor
  currentP = mP.getAngle();
  currentR = mR.getAngle();

  mP.setAngle(Az); // this will always be exactly the azimuth multiplied by the gear ratio
  // in order to calculate the angle of the R gear, we need to add
  // the desired increase in height (relative to the gear ratios) to
  // the angle that the P gear needs to be at
  float extraAngle = Alt / gr_R_Alt; 
  mR.setAngle(Az+extraAngle);

  // now let's figure out what direction to go
  // we can use the current position of the P motor from before
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
  float currentR = mR.getAngle();
  float deltaP = Az - currentP;
  if(deltaP < -180) {
    deltaP += 360;
  } else if(deltaP >= 180) {
    deltaP -= 360;
  }

  float deltaR = (Az + extraAngle) - currentR;
  if(deltaR < -180) {
    deltaR += 360;
  } else if(deltaR >= 180) {
    deltaR -= 360;
  }
  
  Serial.println("DeltaP: " + String(deltaP));
  Serial.println("DeltaR: " + String(deltaR));

  bool oppositeMovement = false; // tracks if we move the motors in opposite directions

  // test to see if we would optimally go opposite directions
  if(deltaR*deltaP < 0) {
    Serial.println("Might need to flip directions...");
    // now make sure we don't have any intersections
    if(deltaP > 0 && Az < currentR) {
      mR.setDirection(!directionP);
      oppositeMovement = true;

    } else if(deltaP < 0 && currentP < (Az + extraAngle)) {
      mR.setDirection(!directionP);
      oppositeMovement = true;

    } else {
      mR.setDirection(directionP);
    }
  } else {
    mR.setDirection(directionP);
  }

  // now we need to decide the speed at which the motors rotate

  // first, check if we need to correct for additional travel made by the R motor when
  // it makes more than 180 deg movement 
  if(deltaR * deltaP <0 && !oppositeMovement) {
    deltaR = abs(360 - abs(deltaR)); // calculate new magnitude of movement for R motor
  }

  // now assign the max movement to whichever motor moves the most
  // first, check to make sure we're moving both
  if(deltaR != 0 && deltaP != 0) {
    if(abs(deltaR) > abs(deltaP)) {
      mR.setSpeed(1);
      mP.setSpeed(deltaP / deltaR);
    } else {
      mP.setSpeed(1);
      mR.setSpeed(deltaR / deltaP);
    }
  } else if(deltaR == 0) {
    mP.setSpeed(1);
  } else {
    mR.setSpeed(1);
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
  // check to see if we're actually tracking something
  if(objectID != 0) {
    struct Horizon newCoords = (*objectObserver).calculatePosition(objectID);
    setAzAlt(newCoords);
  }
}

void Set::trackObject(int ID) {
  this->objectID = ID;
  calculateAzAlt();
}

void Set::setObserver(Observer newObserver) {
  this->objectObserver = &newObserver;
}

void Set::setObjectBank(int newBank[]) {
  for(int i = 0; i < 5; i++) {
    objectBank[i] = newBank[i];
  }
  objectID = objectBank[bankSelection];
  calculateAzAlt();
}

void Set::checkSelection() {
  // only check buttons if we havent had a press in the last 25 ms 
  if(millis() > lastPress + 25) {
    lastPress = millis();
    int selectionDiff = 0; // add or subtract button values to this. Allows simultaneous presses to cancel out easily

    // first, gather the current states of the selection buttons
    bool left_currentState = digitalRead(pin_left);
    bool right_currentState = digitalRead(pin_right);

    // only check for falling edge input (button is HIGH when unpressed)
    if (left_currentState == LOW && left_prevState == HIGH) {
      selectionDiff--;
    }

    if(right_currentState == LOW && right_prevState == HIGH) {
      selectionDiff++;
    }

    // now update the bank selection based on this
    bankSelection += selectionDiff;

    // constrain bank selection to whatever it can actually be
    bankSelection = max(0, bankSelection);
    bankSelection = min(bankSelection, 4);

    // force an update if we'ce changed the selection
    if(selectionDiff != 0) {
      objectID = objectBank[bankSelection];
      calculateAzAlt();
      lastUpdate = millis();
    }

    // finally, update our previous button presses
    left_prevState = left_currentState;
    right_prevState = right_currentState;
  }
}