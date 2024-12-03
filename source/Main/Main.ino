#include "Motor.h"
#include "Set.h"
#include "CONFIG.h"
#include "Observer.h"
// motor definitions

// pin order is:
// EN, STEP, DIR
int pins_1R[3] = { 4, 3, 8 };  // green tape
int pins_1P[3] = { 7, 6, 5 };  // yellow tape
int set1_selectRight = 9;
int set1_selectLeft = 10;

Observer objectTracker;
Set Set1(pins_1P, pins_1R, set1_selectRight, set1_selectLeft, 48, 48, 2);


void setup() {
  Serial.begin(115200);
  Serial.println("hello!");
  Set1.setObserver(objectTracker);

  int objects[5] = {1000,1003,2000,2001,2002};
  Set1.setObjectBank(objects);
}

void loop() {
  Set1.update();
}

void testCircle() {
  struct Horizon testCoords;
  testCoords.azimuth = 90;
  testCoords.altitude = 80;
  Set1.setAzAlt(testCoords);


  while (!Set1.checkMotors()) {
    Set1.update();
  }

  testCoords.azimuth = 180;
  testCoords.altitude = 0;
  Set1.setAzAlt(testCoords);

  while (!Set1.checkMotors()) {
    Set1.update();
  }

  testCoords.azimuth = 270;
  testCoords.altitude = 80;
  Set1.setAzAlt(testCoords);

  while (!Set1.checkMotors()) {
    Set1.update();
  }

  testCoords.azimuth = 0;
  testCoords.altitude = 0;
  Set1.setAzAlt(testCoords);

  while (!Set1.checkMotors()) {
    Set1.update();
  }

  delay(1000);
}