/*
  Balloons | VALBAL | December 2017
  Davy Ragland | dragland@stanford.edu
  Claire Huang | chuang20@stanford.edu
  Keegan Mehall | kmehall@stanford.edu
  Jonathan Zwiebel | jzwiebel@stanford.edu

  File: Actuators.cpp
  --------------------------
  Implimentation of Actuators.h
*/

#include "Actuators.h"


//#define JANKSHITL

//extern float Slift;

void boopBoop() {
	analogWrite(CUTDOWN_FWD, 128);
}
void Boopboop() {
	analogWrite(CUTDOWN_FWD, 0);
}

void dootDoot() {
	analogWrite(CUTDOWN_REV, 128);
}
void Dootdoot() {
	analogWrite(CUTDOWN_REV, 0);
}

#define analogWrite(VALVE_FORWARD, HIGH); val_fwd=0;
#define analogWrite(VALVE_REVERSE, HIGH); val_rev=0;
#define analogWrite(BALLAST_FORWARD, HIGH); bal_fwd=0;
#define analogWrite(BALLAST_REVERSE, HIGH); bal_rev=0;
#define analogWrite(VALVE_FORWARD, LOW); val_fwd=0;
#define analogWrite(VALVE_REVERSE, LOW); val_rev=0;
#define analogWrite(BALLAST_FORWARD, LOW); bal_fwd=0;
#define analogWrite(BALLAST_REVERSE, LOW); bal_rev=0;
#define analogWrite(VALVE_FORWARD, valveMotorSpeedOpenValve); val_fwd=2;
#define analogWrite(VALVE_REVERSE, valveMotorSpeedOpenValve); val_rev=2;
#define analogWrite(VALVE_FORWARD, valveMotorSpeedCloseValve); val_fwd=2;
#define analogWrite(VALVE_REVERSE, valveMotorSpeedCloseValve); val_rev=2;
#define analogWrite(BALLAST_FORWARD, ballastMotorSpeed); bal_fwd=2;
#define analogWrite(BALLAST_REVERSE, ballastMotorSpeed); bal_rev=2;

static IntervalTimer sw_pwm;
static const int n_cts = 10;
int val_duty = 4;
int bal_duty = 6;
static int val_ctr = 0;
static int bal_ctr = 0;
// 0 is off, 1 is high, 2 is pwm
static int val_fwd = 0;
static int val_rev = 0;
static int bal_fwd = 0;
static int bal_rev = 0;

auto pwm_fn = [](){
  val_ctr++; val_ctr = val_ctr % n_cts;
  bal_ctr++; bal_ctr = bal_ctr % n_cts;
  digitalWriteFast(VALVE_FORWARD,(val_fwd==1)||((val_fwd==2)&&(val_ctr<val_duty)));
  digitalWriteFast(VALVE_REVERSE,(val_rev==1)||((val_rev==2)&&(val_ctr<val_duty)));
  digitalWriteFast(BALLAST_FORWARD,(bal_fwd==1)||((bal_fwd==2)&&(bal_ctr<bal_duty)));
  digitalWriteFast(BALLAST_REVERSE,(bal_rev==1)||((bal_rev==2)&&(bal_ctr<bal_duty)));
};

static Encoder balenc(BALLAST_ENCA, BALLAST_ENCB);
static Encoder valenc(VALVE_ENCA, VALVE_ENCB);

/**********************************  SETUP  ***********************************/
/*
 * Function: init
 * -------------------
 * This function initializes the PCB hardware.
 */
void Actuators::init() {
  pinMode(VALVE_FORWARD,    OUTPUT);
  pinMode(VALVE_REVERSE,    OUTPUT);
  pinMode(BALLAST_FORWARD,  OUTPUT);
  pinMode(BALLAST_REVERSE,  OUTPUT);
  pinMode(CUTDOWN_POWER,    OUTPUT);
  pinMode(BALLAST_ENCPWR,   OUTPUT);
  pinMode(VALVE_ENCPWR,     OUTPUT);
  //pinMode(BALLAST_ENCA,     INPUT);
  //pinMode(VALVE_ENCA,       INPUT);
  //pinMode(BALLAST_ENCB,     INPUT);
  //pinMode(VALVE_ENCB,       INPUT);
  //pinMode(CUTDOWN_SIGNAL,   OUTPUT);
  digitalWrite(CUTDOWN_POWER, LOW);
  //digitalWrite(CUTDOWN_SIGNAL, LOW);
  sw_pwm.priority(64);
  sw_pwm.begin(pwm_fn,162);
}

/********************************  FUNCTIONS  *********************************/
/*
 * Function: updateMechanicalConstants
 * -------------------
 * This function updates the mechanical constants.
 */
void Actuators::updateMechanicalConstants(uint16_t valveMotorSpeedOpenValue, uint16_t valveMotorSpeedCloseValue, uint16_t ballastMotorSpeedValue, uint32_t valveOpeningTimeoutValue, uint32_t valveClosingTimeoutValue) {
  valveMotorSpeedOpen = valveMotorSpeedOpenValue;
  valveMotorSpeedClose = valveMotorSpeedCloseValue;
  ballastMotorSpeed = ballastMotorSpeedValue;
  valveOpeningTimeout = valveOpeningTimeoutValue;
  valveClosingTimeout = valveClosingTimeoutValue;
}

/*
 * Function: queueValve
 * -------------------
 * This function increments the timer queue
 * for the mechanical valve mechanism.
 */
void Actuators::queueValve(uint32_t  duration, bool real) {
  if(real) valveQueue += duration;
  else valveQueueFake += duration;
}

/*
 * Function: queueBallast
 * -------------------
 * This function increments the timer queue
 * for the mechanical ballast mechanism.
 */
void Actuators::queueBallast(uint32_t  duration, bool real) {
  Serial.print("Just queued");
  Serial.print(duration);
  Serial.println("ms of ballast");
  if(real) ballastQueue += duration;
  else ballastQueueFake += duration;
}

/*
 * Function: clearValveQueue
 * -------------------
 * This clears any queued valve times.
 */
void Actuators::clearValveQueue() {
  valveQueue = 0;
  valveQueueFake = 0;
}

/*
 * Function: clearBallastQueue
 * -------------------
 * This clears any queued ballast times.
 */
void Actuators::clearBallastQueue() {
  Serial.println("Just cleared ballast queue");
  ballastQueue = 0;
  ballastQueueFake = 0;
}

void Actuators::pause () {
  paused = true;
}

void Actuators::play() {
  paused = false;
}

/*
 * Function: checkValve
 * -------------------
 * This function provides a non-hanging interface to check the timer queue.
 * Called every loop; updates and acts on the current state of the valve.
 */
bool Actuators::checkValve(float current) {
  if (paused) return valveState != CLOSED;
  // Serial.print("Called checkValve with ");
  // Serial.print(valveQueue);
  // Serial.println(" in valveQueue");
  if (valveState == CLOSED) {
    if (valveQueue == 0) {
      uint32_t deltaTime = (millis() - valveCheckTime);
      valveCheckTime = millis();
      (deltaTime >= valveQueueFake) ? (valveQueueFake = 0) : (valveQueueFake -= deltaTime);
    }
    if (valveQueue > 0) {
      val_initial = valenc.read();
      valveActionStartTime = millis();
      valveCheckTime = millis();
      valveState = OPENING;
      openValve();
    }
  }
  if ((valveState == OPENING) && (millis() - valveActionStartTime >= valveOpeningTimeout)) {
    valveState = OPEN;
    stopValve();
	val_delta = valenc.read() - val_initial;
    delta_read = false;
  }
  if (valveState == OPEN) {
    if(valveQueue > 0) {
      uint32_t deltaTime = (millis() - valveCheckTime);
      valveCheckTime = millis();
      (deltaTime >= valveQueue) ? (valveQueue = 0) : (valveQueue -= deltaTime);
      #ifdef JANKSHITL
        Slift -= deltaTime/1000.*0.001;
      #endif
    }
    if(valveQueue == 0) {
      valveActionStartTime = millis();
      valveState = CLOSING;
      closeValve();
    }
  }
  if ((valveState == CLOSING) && (millis() - valveActionStartTime >= valveClosingTimeout)) {
    valveState = CLOSED;
    stopValve();
  }

  valenc_count = valenc.read();
  return valveState != CLOSED;
}

/*
 * Function: checkBallast
 * -------------------
 * This function provides a non-hanging interface to check the timer queue.
 * Called every loop; updates and acts on the current state of the ballast.
 */
bool Actuators::checkBallast(float current, uint32_t reverseTimeout, uint16_t stallCurrent) {
  if (paused) return ballastState != CLOSED;
  // Serial.print("Called checkBallast with ");
  // Serial.print(ballastQueue);
  // Serial.print(" in ballastQueue and direction ");
  // Serial.println(ballastDirection);
  if (ballastState == CLOSED) {
    if (ballastQueue == 0) {
      uint32_t deltaTime = (millis() - ballastCheckTime);
      ballastCheckTime = millis();
      (deltaTime >= ballastQueueFake) ? (ballastQueueFake = 0) : (ballastQueueFake -= deltaTime);
    }
    if (ballastQueue > 0) {
      ballastActionStartTime = millis();
      ballastCheckTime = millis();
      ballastDirectionTime = millis();
      ballastState = OPEN;
    }
  }
  if (ballastState == OPEN) {
    if ((current >= stallCurrent || current <= -stallCurrent) && ((currentLast < stallCurrent) || (millis() - ballastStallTime >= BALLAST_STALL_TIMEOUT)) && (millis() - ballastDirectionTime >= BALLAST_STALL_TIMEOUT)) {
      ballastDirection = !ballastDirection;
      ballastStallTime = millis();
      numBallastOverCurrents++;
    }
    if((millis() - ballastForceReverseTime) >= reverseTimeout) {
      ballastDirection = !ballastDirection;
      ballastForceReverseTime = millis();
    }
    currentLast = current;
    if(ballastQueue > 0) {
      uint32_t deltaTime = (millis() - ballastCheckTime);
      ballastCheckTime = millis();
      (deltaTime >= ballastQueue) ? (ballastQueue = 0) : (ballastQueue -= deltaTime);
      #ifdef JANKSHITL
        Slift += deltaTime/1000.*0.0002;
      #endif
      dropBallast(ballastDirection);
    }
    if(ballastQueue == 0) {
      ballastState = CLOSED;
      stopBallast();
    }
  }
  /*
  long bal_enc = balenc.read();
  bool a1 = digitalRead(BALLAST_ENCA);
  bool a2 = digitalRead(BALLAST_ENCB);
  digitalWrite(VALVE_ENCPWR, 1);
  digitalWrite(BALLAST_ENCPWR, 1);
  bool v1 = digitalRead(VALVE_ENCA);
  bool v2 = digitalRead(VALVE_ENCB);
  Serial.print("Enc = ");
  Serial.println(bal_enc);
  Serial.print(a1);
  Serial.print(", ");
  Serial.println(a2);
  Serial.print(v1);
  Serial.print(", ");
  Serial.println(v2);
  */

  long balenc_count = balenc.read();
  balenc_sum += abs(balenc_count_prev - balenc_count);
  balenc_count_prev = balenc_count;
  return ballastState != CLOSED;
}

/*
 * Function: getValveQueue
 * -------------------
 * This function returns the current valve queue.
 */
uint32_t Actuators::getValveQueue() {
  return valveQueue + valveQueueFake;
}

/*
 * Function: getBallastQueue
 * -------------------
 * This function returns the current ballast queue.
 */
uint32_t Actuators::getBallastQueue() {
  return ballastQueue + ballastQueueFake;
}

/*
 * Function: getNumBallastOverCurrents
 * -------------------
 * This function returns the number of times that the ballast has over currented.
 */
uint32_t Actuators::getNumBallastOverCurrents() {
  return numBallastOverCurrents;
}

/*
 * Function: clearBallastOverCurrents
 * -------------------
 * This function clears the number of times that the ballast has over currented.
 */
void Actuators::clearBallastOverCurrents() {
  numBallastOverCurrents = 0;
}

/*
* Function: getBallastDirection
* ---------------0
* This function returns the current direction of the ballast motor.
*/
bool Actuators::getBallastDirection() {
  return ballastDirection;
}

/*
 * Function: cutDown
 * -------------------
 * This function triggers the mechanical cutdown of the payload.
 */
void Actuators::cutDown() {
  Serial.println("starting cutdown...");
  clearValveQueue();
  clearBallastQueue();
#ifdef MOTOR_CUTDOWN
Serial.println("heyyy doing motor cutdown");
Serial.println(CUTDOWN_FWD);
Serial.println(CUTDOWN_REV);
pinMode(CUTDOWN_FWD, OUTPUT);
pinMode(CUTDOWN_REV, OUTPUT);
	boopBoop();
	delay(2000);
	Boopboop();
	delay(2000);
	dootDoot();
	delay(2000);
	Dootdoot();
#else
	pinMode(CUTDOWN_POWER, OUTPUT);
  digitalWriteFast(CUTDOWN_POWER, HIGH);
  //digitalWrite(CUTDOWN_SIGNAL, HIGH);
  delay(CUTDOWN_DURATION);
  digitalWriteFast(CUTDOWN_POWER, LOW);
  //digitalWrite(CUTDOWN_SIGNAL, LOW);
#endif
  Serial.println("cutdown completed.");
}

/*
 * Function: stopValve
 * -------------------
 * This function stops the valve.
 */
void Actuators::stopValve() {
  digitalWriteFast(VALVE_ENCPWR, 0);
  Serial.println("--- STOPPING VALVE ---");
  val_fwd = 0;
  val_rev = 0;
}

/*********************************  HELPERS  **********************************/
/*
 * Function: openValve
 * -------------------
 * This function starts opening the valve.
 */
void Actuators::openValve() {
  digitalWriteFast(VALVE_ENCPWR, 1);
  Serial.println("--- OPEN VALVE ---");
  val_fwd = 2;
  val_rev = 0;
}

/*
 * Function: closeValve
 * -------------------
 * This function starts closing the valve.
 */
void Actuators::closeValve() {
  Serial.println("--- CLOSE VALVE ---");
  val_fwd = 0;
  val_rev = 2;
}

/*
 * Function: stopBallast
 * -------------------
 * This function stops the ballast.
 */
void Actuators::stopBallast() {
  digitalWriteFast(BALLAST_ENCPWR, 0);
  bal_fwd = 0;
  bal_rev = 0;
}

/*
 * Function: dropBallast
 * -------------------
 * This function drops ballast.
 */
void Actuators::dropBallast(bool direction) {
  digitalWriteFast(BALLAST_ENCPWR, 1);
  if (direction) {
    bal_fwd = 2;
    bal_rev = 0;
  } else {
    bal_fwd = 0;
    bal_rev = 2;
  }
}
