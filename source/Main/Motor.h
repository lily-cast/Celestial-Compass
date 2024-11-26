// this class is made for the TC78H670FTG stepper motor driver. Control
// is given over the desired degrees and angular velocity of the motor
// with the intention to call the update() function in the loop() in main.ino

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class Motor {
  private:
    // pin definitions for physical connection to driver
    int EN_pin;
    int STEP_pin; // aka MODE2
    int DIR_pin; // aka MODE3

    float gear_ratio;

    // timing variables
    unsigned long lastUpdate;

    bool direction;
  public:
    Motor(int pins[], float GR);
    void init();
    void setDirection(bool direction); // 1 is CCW, 0 is CW
    void setAngle(float angle); // allows the user to set a desired angle for the motor to go to
    float getAngle(); // gets the current angle, adjusted by the gearing of the motor
    bool checkAngle(); // checks if the motor is at the desired angle or not (useful for calling update only when required)
    void setSpeed(float deg_sec); // sets the desired speed of the motor in deg/sec
    void update(); // updates the motor position, intended to be called every loop
    
    float stepResolution;

    // motor settings
    void setCurrentLimit(float limit); // sets the current limit 
    void enable(bool onoff); // turns the motor on or off
    void setStepResolution(int res); // sets the step size to be 1:res, must be in a power of 2
    void setGearRatio(float GR); // sets the internal gear ratio

    // angle variables

    // keep track of the actual gearing of the internal motor
    float currentAngle_real;
    float targetAngle_real;

    // keeps track of the gearing as seen by the set
    float currentAngle_geared;
    float targetAngle_geared;

    float maxAngularVelocity; 
    float angularSpeed; // expressed as a decimal from 0 - 1
};

#endif