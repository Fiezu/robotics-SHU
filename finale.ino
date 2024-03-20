#include <Zumo32U4.h>

Zumo32U4ButtonA buttonA;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4Motors motors;
Zumo32U4LCD lcd;
Zumo32U4Buzzer buzzer;

#define NUM_SENSORS 5
#define PROX_SENSOR_LIMIT 6
#define LIMIT_THRESHOLD 800
#define MOTOR_FAST_SPEED 200
#define MOTOR_SPEED 80
#define TURN_SPEED 100
#define DELAY_SPEED_TURN 150
#define DELAY_SPEED_REVERSE 100

#define LEFT_SENSOR 0
#define MIDDLE_LEFT_SENSOR 1
#define MIDDLE_SENSOR 2
#define MIDDLE_RIGHT_SENSOR 3
#define RIGHT_SENSOR 4

struct Movement {
  int startTime;
  int duration;
  int action;  // -1 for left turn, 0 for forward, 1 for right turn, 2 for reverse, 3 for 120-degree turn
};

Movement movements[300];
uint16_t movementCount = 0;
uint16_t houseCount = 0;
bool deliveryMade = false;
bool isRetracing = false;
uint16_t lineSensorValues[NUM_SENSORS];
int turnHistory[4] = { 0, 0, 0, 0 };  // Track the last four turns: 1 for right, 2 for left

void botTurnLeft() {
  motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
}

void botTurnRight() {
  motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
}

void botMoveForward() {
  motors.setSpeeds(MOTOR_SPEED, MOTOR_SPEED);
}

void botReverse() {
  motors.setSpeeds(-MOTOR_FAST_SPEED, -MOTOR_FAST_SPEED);  // Reverse a bit
}

void addTurn(int turnDirection) {
  // Shift the history and add the new turn
  for (int i = 0; i < 3; i++) {
    turnHistory[i] = turnHistory[i + 1];
  }
  turnHistory[3] = turnDirection;
}

bool checkPattern() {
  // Check if the pattern is right-left-right-left or left-right-left-right
  if (turnHistory[0] != 0 && turnHistory[0] == -turnHistory[1] && turnHistory[1] == turnHistory[2] && turnHistory[2] == -turnHistory[3]) {
    return true;
  }
  return false;
}

void startMovement(int action) {
  if (movementCount < sizeof(movements) / sizeof(movements[0])) {
    movements[movementCount].action = action;
    movements[movementCount].startTime = millis();
  }
}

void endMovement() {
  if (movementCount < sizeof(movements) / sizeof(movements[0])) {
    movements[movementCount].duration = millis() - movements[movementCount].startTime;
    movementCount++;
  }
}

void performMovement(int action, int duration) {
  switch (action) {
    case -1:  // Left Turn
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      duration =  DELAY_SPEED_TURN;
      break;
    case 0:  // Forward
      motors.setSpeeds(MOTOR_SPEED, MOTOR_SPEED);
      break;
    case 1:  // Right Turn
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      duration = DELAY_SPEED_TURN;
      break;
    case 2:  // Reverse
      motors.setSpeeds(MOTOR_FAST_SPEED, MOTOR_FAST_SPEED);
      break;
    case 3:  // 120-degree Turn
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      if (duration == 0) duration = 1500;
      break;
  }
  if (duration > 0) delay(duration);  // Perform for a set duration if specified
}

void retraceSteps() {
  if (!isRetracing) {
    isRetracing = true;

    for (int i = movementCount - 1; i >= 0; i--) {
      int reverseAction = movements[i].action;
      if (reverseAction == 1) {
        reverseAction = -1;
      } else if (reverseAction == -1) {
        reverseAction = 1;
      }
      performMovement(reverseAction, movements[i].duration);
      readLineSensors();
    }
  }
  motors.setSpeeds(0, 0);  // Stop at the end
  buzzer.playFromProgramSpace(PSTR("!>>a32"));
  delay(500);

  lcd.clear();
  lcd.print(F("Back at"));
  lcd.gotoXY(0, 1);
  lcd.print(F("start!"));
}

void setup() {
  Serial.begin(9600);
  lineSensors.initFiveSensors();
  proxSensors.initThreeSensors();

  ledYellow(1);

  lcd.clear();
  lcd.print(F("Press A"));
  lcd.gotoXY(0, 1);
  lcd.print(F("to start"));

  // Wait for button A press to start
  while (!buttonA.getSingleDebouncedPress()) {
    delay(10);  // Check for button press every 10ms
  }

  lcd.clear();
  lcd.print(F("Go!"));
  delay(100);
  buzzer.play("L16 cdegreg4");
  while (buzzer.isPlaying());
}

void loop() {
  if (!deliveryMade) {
    exploreMaze();
  } else {
    retraceSteps();
  }
}

bool exploreMaze() {
  ledYellow(0);
  ledGreen(1);
  startMovement(0);
  // Move forward
  botMoveForward();

  // Send IR pulses and read the proximity sensors.
  proxSensors.read();

  // Read the front proximity sensor
  uint16_t leftSensorValue = proxSensors.countsLeftWithLeftLeds();
  uint16_t frontLeftSensorValue = proxSensors.countsFrontWithLeftLeds();
  uint16_t frontRightSensorValue = proxSensors.countsFrontWithRightLeds();
  uint16_t rightSensorValue = proxSensors.countsRightWithRightLeds();

  // lcd.clear();
  // lcd.gotoXY(0, 1);
  // lcd.print(leftSensorValue);
  // lcd.print(" ");
  // lcd.print(frontLeftSensorValue);
  // lcd.print(" ");
  // lcd.print(frontRightSensorValue);
  // lcd.print(" ");
  // lcd.print(rightSensorValue);

  // Determine if an object is visible or not.
  bool objectSeen = (leftSensorValue >= PROX_SENSOR_LIMIT && frontLeftSensorValue >= PROX_SENSOR_LIMIT) || (rightSensorValue >= PROX_SENSOR_LIMIT && frontRightSensorValue >= PROX_SENSOR_LIMIT);

  if (objectSeen) {
    ledYellow(1);

    motors.setSpeeds(0, 0);
    lcd.clear();
    lcd.print(F("House "));
    lcd.gotoXY(0, 1);
    // lcd.print(F("found! "));
    houseCount++;
    lcd.print(houseCount);
    delay(1850);
    // botReverse();
    // delay(DELAY_SPEED);
    botTurnRight();
    delay(1950); // !!! DONT EVER CHANGE THIS !!!!

    // deliveryMade = true;
    // return;
  }

  // Read line sensor values
  readLineSensors();
  endMovement();
}

void readLineSensors() {
  lineSensors.read(lineSensorValues, QTR_EMITTERS_ON);

  uint16_t valueLeftSensor = lineSensorValues[LEFT_SENSOR];
  uint16_t valueLeftMiddleSensor = lineSensorValues[MIDDLE_LEFT_SENSOR];
  uint16_t valueMiddleSensor = lineSensorValues[MIDDLE_SENSOR];
  uint16_t valueRightMiddleSensor = lineSensorValues[MIDDLE_RIGHT_SENSOR];
  uint16_t valueRightSensor = lineSensorValues[RIGHT_SENSOR];

  // If the pattern is detected, execute a 180-degree turn
  if (checkPattern() || valueMiddleSensor >= 600) {
    endMovement();
    execute180Turn(turnHistory[3]);               // Use the last turn direction for the 120-degree turn
    memset(turnHistory, 0, sizeof(turnHistory));  // Reset turn history after executing the turn
  } 
  // else if (valueMiddleSensor >= 600) {
  //   endMovement();
  //   execute120Turn(turnHistory[3]);               // Use the last turn direction for the 120-degree turn
  //   memset(turnHistory, 0, sizeof(turnHistory));  // Reset turn history after executing the turn
  //   // Code to turn right
  //   // startMovement(2);
  //   // botReverse();
  //   // delay(DELAY_SPEED_REVERSE);
  //   // endMovement();
  //   // startMovement(3);
  //   // botTurnRight();
  //   // delay(1000);
  // } 
  else if (valueLeftSensor >= LIMIT_THRESHOLD && valueLeftMiddleSensor >= LIMIT_THRESHOLD) {
    endMovement();
    // Code to turn right
    startMovement(2);
    botReverse();
    delay(DELAY_SPEED_REVERSE);
    endMovement();
    startMovement(1);
    addTurn(1);
    botTurnRight();
    delay(DELAY_SPEED_TURN);
  } else if (valueRightSensor >= LIMIT_THRESHOLD && valueRightMiddleSensor >= LIMIT_THRESHOLD) {
    endMovement();
    // Code to turn left
    startMovement(2);
    botReverse();
    delay(DELAY_SPEED_REVERSE);
    endMovement();
    startMovement(-1);
    addTurn(-1);
    botTurnLeft();
    delay(DELAY_SPEED_TURN);
  } 
}

// Execute a 180-degree turn based on the last turn direction
void execute180Turn(int lastTurnDirection) {
  startMovement(2);
  botReverse();
  delay(DELAY_SPEED_REVERSE);
  endMovement();
  if (lastTurnDirection == 1) {  // Last turn was right
    startMovement(3);
    botTurnLeft();
  } else {  // Last turn was left
    startMovement(3);
    botTurnRight();
  }
  delay(1500);  // Adjust this delay to achieve approximately 120 degrees turn
}
