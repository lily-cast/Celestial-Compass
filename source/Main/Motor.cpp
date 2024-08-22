#include "Motor.h"
#include "SparkFun_ProDriver_TC78H670FTG_Arduino_Library.h"


Motor::Motor(int STBY_pin, int EN_pin, int MODE0_pin, int MODE1_pin, int STEP_pin, int DIR_pin, int ERR_pin) {
    // store intialized pins as local variables
    this->STBY_pin = STBY_pin;
    this->EN_pin = EN_pin;
    this->MODE0_pin = MODE0_pin;
    this->MODE1_pin = MODE1_pin;
    this->STEP_pin = STEP_pin;
    this->DIR_pin = DIR_pin;
    this->ERR_pin = ERR_pin;

    init();
}

void Motor::init() {
    // set the state of all required pins
    motorDriver.settings.standbyPin = STBY_pin;
    motorDriver.settings.enablePin = EN_pin;
    motorDriver.settings.mode0Pin = MODE0_pin;
    motorDriver.settings.mode1Pin = MODE1_pin;
    motorDriver.settings.mode2Pin = STEP_pin;

}