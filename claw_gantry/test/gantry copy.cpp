#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

#define LIMIT_SWITCH_A 11
#define LIMIT_SWITCH_B 12
#define LIMIT_SWITCH_C 10

#define STEP_A 3
#define DIR_A 2

#define STEP_B 5
#define DIR_B 4

#define STEP_C 6
#define DIR_C 7

#define SPEED 2000

#define OUTER_DIM 4600
#define INNER_DIM 4000

void go_outer_target(long pos);
void set_outer_target(long pos);
void set_target(int scale[2]);
void zero();

bool limit_found = false;

AccelStepper stepperA(AccelStepper::DRIVER, STEP_A, DIR_A);
AccelStepper stepperB(AccelStepper::DRIVER, STEP_B, DIR_B);
MultiStepper outer_axis;

AccelStepper inner_axis(AccelStepper::DRIVER, STEP_C, DIR_C);


void setup() {
  
  Serial.begin(115200);

  pinMode(LIMIT_SWITCH_A, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_B, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_C, INPUT_PULLUP);

  stepperA.setMaxSpeed(SPEED);

  stepperB.setMaxSpeed(SPEED);

  inner_axis.setMaxSpeed(SPEED);
  inner_axis.setAcceleration(SPEED);

  outer_axis.addStepper(stepperA);
  outer_axis.addStepper(stepperB);

  stepperA.setCurrentPosition(0);
  stepperB.setCurrentPosition(0);
  inner_axis.setCurrentPosition(0);

  zero();

  // set_outer_target(2300);
  // inner_axis.moveTo(2000);
}

void loop() {
  outer_axis.run();
  inner_axis.run();

  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 's') {
      // Serial.println("Received 's'");
      int scale[2];
      int i = 0;
      char c;
      while (c != 'e') { 
        while (Serial.available() == 0){}
           
        c = Serial.read();
        Serial.println(c);
        if (c == 'e') {
          // Serial.println("Received e");
          set_target(scale);
          Serial.println("Set target");
          break;
        }
        scale[i] = uint8_t(c);
        i++;
        if (i == 4) {
          break;
        }
    }
    Serial.println("Done");
  }
  
  digitalWrite(13, LOW);
}
}

void zero() {
  set_outer_target(-5000);
  while (!limit_found)
  {
     
  if (digitalRead(LIMIT_SWITCH_A) == 0 && !limit_found || digitalRead(LIMIT_SWITCH_B) == 0 && !limit_found) {
    while (digitalRead(LIMIT_SWITCH_A) == 1)
    {
      stepperA.setSpeed(SPEED/4);
      stepperA.runSpeed();
      Serial.println("A");
    }
    stepperA.stop();
    stepperA.setCurrentPosition(0);

    while (digitalRead(LIMIT_SWITCH_B) == 1)
    {
      stepperB.setSpeed(-SPEED/4);
      stepperB.runSpeed();
      Serial.println("B");
    }
    stepperB.stop();
    stepperB.setCurrentPosition(0);

    stepperA.setSpeed(SPEED);
    stepperB.setSpeed(SPEED);
    delay(100);
    limit_found = true;
  }
  if (limit_found)
  {
    break;
  }
  
  outer_axis.run();
  }
  inner_axis.moveTo(-5000);
  while (digitalRead(LIMIT_SWITCH_C) == 1)
    {
      inner_axis.setSpeed(-SPEED/4);
      inner_axis.runSpeed();
      Serial.println("Inner");
    }
  inner_axis.stop();
  inner_axis.setCurrentPosition(0);
}

void set_outer_target(long pos) {
  long positions[2]; // Array of desired stepper positions
  positions[0] = -pos;
  positions[1] = pos;
  outer_axis.moveTo(positions);
}

void set_target(int scale[2]) {
  
  Serial.print("Scale: ");
  for (int i = 0; i < 2; i++) {
    Serial.print(scale[i]);
    Serial.print(" ");
  }
  Serial.println();

  float pos[2];
  pos[0] = constrain(float(float(scale[0])/255.0) * OUTER_DIM, 0, OUTER_DIM);
  pos[1] = constrain(float(float(scale[1])/255.0) * INNER_DIM, 0, INNER_DIM);

  Serial.print("Pos: ");
  for (int i = 0; i < 2; i++) {
    Serial.print(pos[i]);
    Serial.print(" ");
  }

  set_outer_target(pos[0]);
  inner_axis.moveTo(pos[1]);
}