#include "Motor.h"
#include "CONFIG.h"


Motor::Motor(int STBY_pin, int EN_pin, int MODE0_pin, int MODE1_pin, int STEP_pin, int DIR_pin) {
    // store intialized pins as local variables
    this->STBY_pin = STBY_pin;
    this->EN_pin = EN_pin;
    this->MODE0_pin = MODE0_pin;
    this->MODE1_pin = MODE1_pin;
    this->STEP_pin = STEP_pin; // also MODE2
    this->DIR_pin = DIR_pin; // also MODE3

    init();
}

void Motor::init() {
    // set the output mode of all the motor pins
    pinMode(STBY_pin, OUTPUT);
    pinMode(EN_pin, OUTPUT);
    pinMode(MODE0_pin, OUTPUT);
    pinMode(MODE1_pin, OUTPUT);
    pinMode(STEP_pin, OUTPUT);
    pinMode(DIR_pin, OUTPUT);

    // make sure we're in standby
    digitalWrite(STBY_pin, LOW);

    // set the step resolution to 1/32
    digitalWrite(MODE0_pin, HIGH);
    digitalWrite(MODE1_pin, LOW);
    digitalWrite(STEP_pin, HIGH);
    digitalWrite(DIR_pin, HIGH);

    stepResolution = ((float)1/(float)32);

    // now we release standby and enable the motor to get the motor going
    digitalWrite(STBY_pin, HIGH);
    digitalWrite(EN_pin, HIGH);    

    angularVelocity = 50; // the default deg/s that we want to use
    lastUpdate = millis();
    currentAngle = 0;
}

float Motor::getAngle() {
  return currentAngle;
}

bool Motor::checkAngle(float target) {
  // checks if the current angle is within a step size of the target
  return abs(targetAngle-currentAngle) <= (STEP_SIZE * stepResolution);
}

void Motor::setAngle(float angle) {
  this->targetAngle = angle;
}

void Motor::update() {
  // first, let's make sure we're already at our angle, or at least within a single step of it
  if(abs(targetAngle-currentAngle) >= (STEP_SIZE * stepResolution)) {
    // we'll define positive angles as positive from the right hand rule. So if the target is
    // 15 deg and we're at 0 deg, rotate the motor CCW
  
    // to start, we want to make sure we aren't going too fast. We track the last time we did
    // an update and we also have the deg/s that's desired.

    // so the first thing we do when updating is make sure we *should* be updating
    float millisecsToStep = ((STEP_SIZE * stepResolution)/angularVelocity) * 1000;
    if(millis()-lastUpdate >= millisecsToStep) {
      // reset the timer
      lastUpdate = millis();
      // now we check what direction to go
      if(targetAngle > currentAngle) {
        // need to rotate CCW
        digitalWrite(DIR_pin, LOW);

        // step the motor
        digitalWrite(STEP_pin, HIGH);
        digitalWrite(STEP_pin, LOW);

        currentAngle += STEP_SIZE * stepResolution;
      } else {
        // rotate CW
        digitalWrite(DIR_pin, HIGH);

        // step the motor
        digitalWrite(STEP_pin, HIGH);
        digitalWrite(STEP_pin, LOW);

        currentAngle -= STEP_SIZE * stepResolution;
      }
      Serial.println(currentAngle);
    }
  }
}