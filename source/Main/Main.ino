#include "Motor.h"
#include "Set.h"
#include "CONFIG.h"
// motor definitions

// pin order is:
// stby, en, mode0, mode1, step, dir
int pins_1P[6] = {4, 9, 7, 8, 3, 2}; // green wires
int pins_1R[6] = {15, 21, 19, 20, 14, 16}; // blue wires

Set Set1(pins_1P, pins_1R, 1,1,3);
//Motor test1R(pins_1R);
void setup() {
    // initialize serial and create our motor
    Serial.begin(115200);
    Set1.setAzAlt(180,90);
}

void loop() {
  Set1.update();
}