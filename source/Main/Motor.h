#ifndef MY_MOTOR_H
#define MY_MOTOR_H

#include <Arduino.h>
#include "SparkFun_ProDriver_TC78H670FTG_Arduino_Library.h"

class Motor {
    private:
        // pin definitions for physical connection to driver
        int STBY_pin;
        int EN_pin;
        int MODE0_pin;
        int MODE1_pin;
        int STEP_pin; // aka MODE2
        int DIR_pin; // aka MODE3
        int ERR_pin;

        int currentAngle;
        int targetAngle;
        PRODRIVER motorDriver;

    public:
        Motor(int STBY_pin, int EN_pin, int MODE0_pin, int MODE1_pin, int STEP_pin, int DIR_pin, int ERR_pin);
        void init();
        void set_angle(int angle);
        void get_angle();
        void halt_motor(bool state);
        void update();
};

#endif