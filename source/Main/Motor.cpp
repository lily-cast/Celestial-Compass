#include "Motor.h"

Motor::Motor(byte pin) {
    this->pin = pin;
    init();
}

void Motor::init() {
    digitalWrite(pin, LOW);
}