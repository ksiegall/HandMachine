#include <Arduino.h>
#include <AccelStepper.h>
#include <Servo.h>

#define servoPin 10
#define OPEN 50
#define CLOSE 100

#define LIMIT_SWITCH_A A1
#define LIMIT_SWITCH_B A0
#define LIMIT_SWITCH_C A2

#define STEP_A 3
#define DIR_A 2

#define STEP_B 5
#define DIR_B 4

#define STEP_C 6
#define DIR_C 7

#define STEP_W 9
#define DIR_W 8

#define SPEED 2000
#define WINCH_SPEED 1000

#define OUTER_DIM 4600
#define INNER_DIM 4000

#define CLAW_DROP -1000

int prize_loc[2] = {OUTER_DIM, 0};

void go_outer_target(long pos);
void set_outer_target(long pos);
void set_target(int scale[2]);
void zero();
void run_outer();
void grab();
void serialFlush(); 

bool limit_found = false;

AccelStepper stepperA(AccelStepper::DRIVER, STEP_A, DIR_A);
AccelStepper stepperB(AccelStepper::DRIVER, STEP_B, DIR_B);

AccelStepper inner_axis(AccelStepper::DRIVER, STEP_C, DIR_C);

AccelStepper winch(AccelStepper::DRIVER, STEP_W, DIR_W);

Servo claw;

void setup() {
  
  Serial.begin(115200);

  claw.attach(servoPin);

  pinMode(LIMIT_SWITCH_A, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_B, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_C, INPUT_PULLUP);

  stepperA.setMaxSpeed(SPEED);
  stepperA.setAcceleration(SPEED);

  stepperB.setMaxSpeed(SPEED);
  stepperB.setAcceleration(SPEED);

  inner_axis.setMaxSpeed(SPEED);
  inner_axis.setAcceleration(SPEED);

  winch.setMaxSpeed(WINCH_SPEED);
  winch.setAcceleration(WINCH_SPEED);

  stepperA.setCurrentPosition(0);
  stepperB.setCurrentPosition(0);
  inner_axis.setCurrentPosition(0);

  winch.setCurrentPosition(0);
  // winch.moveTo(300);
  // winch.runToPosition();


  claw.write(OPEN);

  zero();

  grab();

  set_outer_target(2300);
  inner_axis.moveTo(2000);
}

void loop() {
  // outer_axis.run();
  run_outer();
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
  
  // outer_axis.run();
  run_outer();
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
  // outer_axis.moveTo(positions);
  stepperA.moveTo(positions[0]);
  stepperB.moveTo(positions[1]);
}

void set_target(int scale[2]) {
  Serial.print("Scale: ");
  for (int i = 0; i < 2; i++) {
    Serial.print(scale[i]);
    Serial.print(" ");
  }
  Serial.println();

  if (scale[2] == 1) {
    grab();
    return;
  }

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

void run_outer() {

  stepperA.run();
  stepperB.run();
}

void grab() {
  claw.write(OPEN);
  winch.moveTo(CLAW_DROP);
  winch.runToPosition();
  claw.write(CLOSE);
  winch.moveTo(0);
  winch.runToPosition();
  set_target(prize_loc);
  while (stepperA.distanceToGo() != 0 || stepperB.distanceToGo() != 0 || inner_axis.distanceToGo() != 0)
  {
    run_outer();
    inner_axis.run();
  }
  claw.write(OPEN);
  delay(1000);
  Serial.flush();
  serialFlush();

}

void serialFlush(){
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}