#ifndef MY_MOTOR_H
#define MY_MOTOR_H

#include <Arduino.h>

class Motor {
    private:
        byte pin;

    public:
        Motor(byte pin);
        void init();
};

#endif