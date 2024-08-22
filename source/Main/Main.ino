#include "Motor.h"
#include "CONFIG.h"
// motor definitions

Motor m_1R(4, 9, 7, 8, 3, 2); // all the pins for this motor occupy D2 - D9 on the nano

void setup() {
    // initialize serial and create our motor
    Serial.begin(115200);
    m_1R.setAngle(360);
}

void loop() {
    m_1R.update();
}