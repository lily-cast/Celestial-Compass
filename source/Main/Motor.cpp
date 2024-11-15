// this class is made for the TC78H670FTG stepper motor driver. Control
// is given over the desired degrees and angular velocity of the motor
// with the intention to call the update() function in the loop() in main.ino

#include "Motor.h"
#include "CONFIG.h"


Motor::Motor(int pins[], float GR) {
    // store intialized pins as local variables
    this->STBY_pin = pins[0];
    this->EN_pin = pins[1];
    this->MODE0_pin = pins[2];
    this->MODE1_pin = pins[3];
    this->STEP_pin = pins[4]; // also MODE2
    this->DIR_pin = pins[5]; // also MODE3
    this->VREF_pin = pins[6];

    this->gear_ratio = GR;
    init();
};

void Motor::init() {
    // set the output mode of all the motor pins
    pinMode(STBY_pin, OUTPUT);
    pinMode(EN_pin, OUTPUT);
    pinMode(MODE0_pin, OUTPUT);
    pinMode(MODE1_pin, OUTPUT);
    pinMode(STEP_pin, OUTPUT);
    pinMode(DIR_pin, OUTPUT);
    //pinMode(VREF_pin, OUTPUT);

    // make sure we're in standby
    digitalWrite(STBY_pin, LOW);

    // set the step resolution to 1/32
    setStepResolution(8);

    // now we release standby and enable the motor to get the motor going
    digitalWrite(STBY_pin, HIGH);
    enable(true);

    // start with a .6 A limit on the motor
    // 3.3V is equal to max current
    float desiredV = (float) 0.6 / (float) 1.1;

    maxAngularVelocity = 10; // the default deg/s that we want to use
    lastUpdate = millis();
    currentAngle_real = 0;
    currentAngle_geared = 0;
    direction = 1; //  default to CCW
}

float Motor::getAngle() {
  return currentAngle_geared;
}

void Motor::setSpeed(float newSpeed) {
  this->angularSpeed = newSpeed;
}

bool Motor::checkAngle() {
  // checks if the current angle is within a step size of the target, useful for while loops and the like
  //Serial.println(abs(targetAngle_real-currentAngle_real) <= (STEP_SIZE * stepResolution));
  return abs(targetAngle_real-currentAngle_real) <= (STEP_SIZE * stepResolution);
}

void Motor::setAngle(float angle) {
  // correct for anything out of 0-360
  while(angle < 0) {
    angle += 360;
  }
  while(angle > 360) {
    angle -= 360;
  }
  this->targetAngle_real = angle;
  this->targetAngle_geared = angle * gear_ratio;
}

void Motor::setDirection(bool direction) {
  this->direction = direction;
}

void Motor::update() {
  // first, let's make sure we're already at our angle, or at least within a single step of it
  if(abs(targetAngle_real-currentAngle_real) >= (STEP_SIZE * stepResolution)) {
    enable(1);
    // we'll define positive angles as positive from the right hand rule. So if the target is
    // 15 deg and we're at 0 deg, rotate the motor CCW
  
    // to start, we want to make sure we aren't going too fast. We track the last time we did
    // an update and we also have the deg/s that's desired.

    // so the first thing we do when updating is make sure we *should* be updating
    float millisecsToStep = ((STEP_SIZE * stepResolution)/(maxAngularVelocity * angularSpeed)) * 1000;
    if(millis()-lastUpdate >= millisecsToStep) {
      // reset the timer
      lastUpdate = millis();
      // now we check what direction to go
      if(direction) {
        // need to rotate CCW
        digitalWrite(DIR_pin, HIGH);

        // step the motor
        digitalWrite(STEP_pin, HIGH);
        digitalWrite(STEP_pin, LOW);

        currentAngle_real += STEP_SIZE * stepResolution;
        if(currentAngle_real >= 360 * gear_ratio) {
          currentAngle_real -= 360 * gear_ratio;
        }

        currentAngle_geared = currentAngle_real * gear_ratio;
      } else {
        // rotate CW
        digitalWrite(DIR_pin, LOW);

        // step the motor
        digitalWrite(STEP_pin, HIGH);
        digitalWrite(STEP_pin, LOW);

        currentAngle_real -= STEP_SIZE * stepResolution;
        if(currentAngle_real < 0) {
          currentAngle_real += 360*gear_ratio;
        }

        currentAngle_geared = currentAngle_real * gear_ratio;
      }
    }
  } else {
    enable(0);
  }
}

void Motor::enable(bool onoff) {
  digitalWrite(EN_pin, onoff);
}

void Motor::setStepResolution(int res) {
  // TODO: store pin inputs in an array and only call digital write outside of the switch case

  // runs the motor through the standby on/off process, resetting the step size in the proccess
  // first, turn on stanby mode

  digitalWrite(STBY_pin, LOW); // this is active on the low

  // next, run through each case of what resolutions we can do
  switch(res) {
    case 1: 
      // H, L, L, L
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, LOW);
      digitalWrite(MODE1_pin, LOW);
      digitalWrite(MODE0_pin, LOW);
      break;

    case 2:
      // H, L, L, H
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, LOW);
      digitalWrite(MODE1_pin, LOW);
      digitalWrite(MODE0_pin, HIGH);
      break;

    case 4:
      // H, L, H, L
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, LOW);
      digitalWrite(MODE1_pin, HIGH);
      digitalWrite(MODE0_pin, LOW);
      break;

    case 8:
      // H, L, H, H
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, LOW);
      digitalWrite(MODE1_pin, HIGH);
      digitalWrite(MODE0_pin, HIGH);
      break;

    case 16:
      // H, H, L, L
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, HIGH);
      digitalWrite(MODE1_pin, LOW);
      digitalWrite(MODE0_pin, LOW);
      break;
      
    case 32:
      // H, H, L, H
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, HIGH);
      digitalWrite(MODE1_pin, LOW);
      digitalWrite(MODE0_pin, HIGH);
      break;

    case 64:
      // H, H, H, L
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, HIGH);
      digitalWrite(MODE1_pin, HIGH);
      digitalWrite(MODE0_pin, LOW);
      break;

    case 128:
      // H, H, H, H
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, HIGH);
      digitalWrite(MODE1_pin, HIGH);
      digitalWrite(MODE0_pin, HIGH);
      break;

    default:
      // just do full step resolution
      // H, L, L, L
      digitalWrite(DIR_pin, HIGH);
      digitalWrite(STEP_pin, LOW);
      digitalWrite(MODE1_pin, LOW);
      digitalWrite(MODE0_pin, LOW);
      
      // forcibly set the res variable to 1, for use when calculating step size
      res = 1;
      break;
  }
  // disable standby
  digitalWrite(STBY_pin, HIGH);

  // reset the step size variable based on the provided resolution
  // reset step resolution variable
      stepResolution = ((float)1/(float)res);
}