#include "Motor.h"
#include "Set.h"
#include "CONFIG.h"
#include "Observer.h"
// motor definitions

// pin order is:
// EN, STEP, DIR
int pins_1R[3] = {4,3,2}; // green tape
int pins_1P[3] = {7,6,5}; // yellow tape

Set Set1(pins_1P, pins_1R, 32,32,3);
Observer Observer1;
void setup() {
  Serial.begin(115200);
  Serial.println("hello!");
  Observer1.calculatePosition(10);

  Set1.setAzAlt(179, 17);

  
  while(!Set1.checkMotors()) {
    Set1.update();
  }
  Serial.println("Done!");
}

void loop() {

}