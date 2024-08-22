#include "Motor.h"
#include "SparkFun_ProDriver_TC78H670FTG_Arduino_Library.h"
// motor definitions
//Motor m_1R(4, 9, 7, 8, 3, 2, 6); // all the pins for this motor occupy D2 - D9 on the nano


int en_pin = 9;
int stby_pin = 4;
int mode0_pin = 7;
int mode1_pin = 8;
int dir_pin = 2;
int step_pin = 3;

void setup() {
    // initialize serial and create our motor
    Serial.begin(115200);
    
    pinMode(en_pin, OUTPUT);
    pinMode(stby_pin, OUTPUT);
    pinMode(mode0_pin, OUTPUT);
    pinMode(mode1_pin, OUTPUT);
    pinMode(dir_pin, OUTPUT);
    pinMode(step_pin, OUTPUT);
    //m_1R.set_angle(90);
}

void loop() {
    //m_1R.update();
    int i;
    // set clk-in mode and resolution

    // pin settings for full step resolution
    digitalWrite(mode0_pin, HIGH);
    digitalWrite(mode1_pin, LOW);
    digitalWrite(step_pin, HIGH);
    digitalWrite(dir_pin, HIGH);

    digitalWrite(stby_pin, HIGH); // release stanby

    delay(5);

    digitalWrite(mode0_pin, LOW);
    digitalWrite(mode1_pin, LOW);
    digitalWrite(step_pin, LOW);
    digitalWrite(dir_pin, LOW); // set back to default status

    // enable motor
    digitalWrite(en_pin, HIGH);

    // set step resolution

    for (i = 0; i < 1536; i++) {
      digitalWrite(step_pin, LOW);
      digitalWrite(step_pin, HIGH);
      delay(2);
    }

    delay (1000);
}