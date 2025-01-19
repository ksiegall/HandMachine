#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

#define LIMIT_SWITCH 8

#define STEP_A 9
#define DIR_A 2

#define STEP_B 5
#define DIR_B 4

#define STEP_C 6
#define DIR_C 7

#define SPEED 4000

bool limit_found = false;


AccelStepper stepperA(AccelStepper::DRIVER, STEP_A, DIR_A);
AccelStepper stepperB(AccelStepper::DRIVER, STEP_B, DIR_B);
MultiStepper outer_axis;

AccelStepper inner_axis(AccelStepper::DRIVER, STEP_C, DIR_C);


void setup() {
  Serial.begin(9600);
  stepperA.setMaxSpeed(SPEED);
  stepperA.setAcceleration(SPEED/2);
  stepperB.setMaxSpeed(SPEED);
  stepperB.setAcceleration(SPEED/2);

  inner_axis.setMaxSpeed(SPEED);
  inner_axis.setAcceleration(SPEED/2);

  outer_axis.addStepper(stepperA);
  outer_axis.addStepper(stepperB);
}

void loop() {
  long positions[2]; // Array of desired stepper positions
  
  positions[0] = 1000;
  positions[1] = 50;
  outer_axis.moveTo(positions);
  outer_axis.runSpeedToPosition(); // Blocks until all are in position
  delay(1000);
  
  // Move to a different coordinate
  positions[0] = -1000;
  positions[1] = 1000;
  outer_axis.moveTo(positions);
  outer_axis.runSpeedToPosition(); // Blocks until all are in position
  delay(1000);
}