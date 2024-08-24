// this class is made for the TC78H670FTG stepper motor driver. Control
// is given over the desired degrees and angular velocity of the motor
// with the intention to call the update() function in the loop() in main.ino

#ifndef MOTOR_H
#define MOTOR_H

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

    bool direction;
  public:
    Motor(int pins[]);
    void init();
    void setDirection(bool direction); // 1 is CCW, 0 is CW
    void setAngle(float angle); // allows the user to set a desired angle for the motor to go to
    float getAngle(); // gets the current angle
    bool checkAngle(); // checks if the motor is at the desired angle or not (useful for calling update only when required)
    void setSpeed(float deg_sec); // sets the desired speed of the motor in deg/sec
    void update(); // updates the motor position, intended to be called every loop
    
    float stepResolution;

    // angle variables
    float currentAngle;
    float targetAngle;
    float angularVelocity; 
};

#endif