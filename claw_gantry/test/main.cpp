#include <Arduino.h>
#include <AccelStepper.h>

#define LIMIT_SWITCH 8

#define STEP 3
#define DIR 2

#define SPEED 4000

bool limit_found = false;


AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR);

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(SPEED);
  stepper.setAcceleration(SPEED);
  
  
  pinMode(LIMIT_SWITCH, INPUT_PULLUP);
  
  // stepper.setCurrentPosition(0);
  // stepper.moveTo(2000);
  // stepper.runSpeedToPosition();
  stepper.setCurrentPosition(0);
  // stepper.moveTo(1000);
  stepper.moveTo(5000);
  // delay(000);
  // stepper.setSpeed(SPEED);
}

void loop() {
  stepper.run();
  // Serial.println(stepper.currentPosition());


  if (digitalRead(LIMIT_SWITCH) == 1 && !limit_found) {
    stepper.stop();
    stepper.setCurrentPosition(0);
    delay(100);
    stepper.moveTo(-4000);
    limit_found = true;
  }
  // delay(100000);
  // if (stepper.distanceToGo() == 0) {
  //   stepper.moveTo(-stepper.currentPosition());
  // }
  // stepper.run();
}