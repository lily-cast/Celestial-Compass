#ifndef MY_MOTOR_H
#define MY_MOTOR_H

#include <Arduino.h>

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

        // timing variables
        unsigned long lastUpdate;
    public:
        Motor(int STBY_pin, int EN_pin, int MODE0_pin, int MODE1_pin, int STEP_pin, int DIR_pin);
        void init();
        void setAngle(float angle);
        float getAngle();
        bool checkAngle();
        void setSpeed(float deg_sec);
        void update();
        
        float stepResolution;

        // angle variables
        float currentAngle;
        float targetAngle;
        float angularVelocity; 
};

#endif