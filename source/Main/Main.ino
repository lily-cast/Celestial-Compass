#include "Motor.h"
#include "Set.h"
#include "CONFIG.h"
#include "Observer.h"
// motor definitions

// pin order is:
// stby, en, mode0, mode1, step, dir
int pins_1P[7] = {4, 9, 7, 8, 3, 2, A4}; // green wires
int pins_1R[7] = {15, 21, 19, 20, 14, 16, A3}; // blue wires

Set Set1(pins_1P, pins_1R, 1,1,3);
Observer Observer1;
//Motor test1R(pins_1R);
void setup() {
  Serial.begin(115200);
  Serial.println("hello!");
  Observer1.calculatePosition(10);
}

void loop() {

}