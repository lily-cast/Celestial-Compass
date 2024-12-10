// this class is made for the TC78H670FTG stepper motor driver. Control
// is given over the desired degrees and angular velocity of the motor
// with the intention to call the update() function in the loop() in main.ino

#include "Motor.h"
#include "CONFIG.h"
#include "Observer.h"


Motor::Motor(int pins[], float GR) {
    // store intialized pins as local variables
    this->EN_pin = pins[0];
    this->STEP_pin = pins[1]; 
    this->DIR_pin = pins[2];

    this->gear_ratio = GR;
    init();
};

void Motor::init() {
    // set the output mode of all the motor pins
    pinMode(EN_pin, OUTPUT);
    pinMode(STEP_pin, OUTPUT);
    pinMode(DIR_pin, OUTPUT);

    maxAngularVelocity = 5; // the default deg/s that we want to use
    lastUpdate = millis();
    currentAngle_real = 0;
    currentAngle_geared = 0;
    direction = 1; //  default to CCW
    stepResolution = 1;
}

float Motor::getAngle() {
  return currentAngle_geared;
}

void Motor::setSpeed(float newSpeed) {
  // for some weird reason using abs() when setting the speed causes it to come out as 0.. so lets just check for negatives here instead
  if(newSpeed < 0) {
    newSpeed = float(-1) * newSpeed;
  }
  this->angularSpeed = newSpeed;
}

bool Motor::checkAngle() {
  // checks if the current angle is within a step size of the target, useful for while loops and the like
  //Serial.println(abs(targetAngle_real-currentAngle_real) <= (STEP_SIZE * stepResolution));
  return abs(targetAngle_geared-currentAngle_geared) <= (STEP_SIZE * stepResolution);
}

void Motor::setAngle(float angle) {
  // correct for anything out of 0-360
  while(angle < 0) {
    angle += 360;
  }
  while(angle > 360) {
    angle -= 360;
  }
  this->targetAngle_geared = angle;
  this->targetAngle_real = angle * gear_ratio;
}

void Motor::setDirection(bool direction) {
  this->direction = direction;
}

void Motor::update() {
  // first, let's make sure we're already at our angle, or at least within a single step of it
  if(abs(targetAngle_real-currentAngle_real) >= (STEP_SIZE)) {
    // we'll define positive angles as positive from the right hand rule. So if the target is
    // 15 deg and we're at 0 deg, rotate the motor CCW
  
    // to start, we want to make sure we aren't going too fast. We track the last time we did
    // an update and we also have the deg/s that's desired.

    // so the first thing we do when updating is make sure we *should* be updating
    float millisecsToStep = ((STEP_SIZE / gear_ratio)/(maxAngularVelocity * angularSpeed)) * 1000;
    if(millis()-lastUpdate >= millisecsToStep) {
      enable(1);
      // reset the timer
      lastUpdate = millis();
      // now we check what direction to go
      if(direction) {
        // need to rotate CCW
        digitalWrite(DIR_pin, HIGH);

        // step the motor
        digitalWrite(STEP_pin, HIGH);
        digitalWrite(STEP_pin, LOW);

        currentAngle_real += STEP_SIZE;
        if(currentAngle_real >= 360 * gear_ratio) {
          currentAngle_real -= 360 * gear_ratio;
        }

        currentAngle_geared = currentAngle_real / gear_ratio;
      } else {
        // rotate CW
        digitalWrite(DIR_pin, LOW);

        // step the motor
        digitalWrite(STEP_pin, HIGH);
        digitalWrite(STEP_pin, LOW);

        currentAngle_real -= STEP_SIZE;
        if(currentAngle_real < 0) {
          currentAngle_real += 360*gear_ratio;
        }

        currentAngle_geared = currentAngle_real / gear_ratio;
      }
      enable(0);
      //Serial.print("Step percent: ");
      //Serial.print((100.0*currentAngle_geared)/targetAngle_geared, 2);
      //Serial.println("%");
    }
  } else {
    enable(0);
  }
}

void Motor::enable(bool onoff) {
  digitalWrite(EN_pin, onoff);
}
