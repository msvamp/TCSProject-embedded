#include <AFMotor.h>
#include <NewPing.h>

#define MAX_SPEED 190 // sets speed of DC motors

AF_DCMotor motor1(1, MOTOR12_1KHZ);
AF_DCMotor motor2(2, MOTOR12_1KHZ);
AF_DCMotor motor3(3, MOTOR34_1KHZ);
AF_DCMotor motor4(4, MOTOR34_1KHZ);

boolean goesForward=false;
int speedSet = 0;

// the function to stop all 4 motor driver
void moveStop() {
	motor1.run(RELEASE);
	motor2.run(RELEASE);
	motor3.run(RELEASE);
	motor4.run(RELEASE);
}

// code to start moving the motor
void moveForward() {
	if(!goesForward) {
		goesForward=true;
		motor1.run(FORWARD);
		motor2.run(FORWARD);
		motor3.run(FORWARD);
		motor4.run(FORWARD);
		for (speedSet = 0; speedSet < MAX_SPEED; speedSet +=4) {
			// slowly bring the speed up to avoid loading down the batteries too quickly
			motor1.setSpeed(speedSet);
			motor2.setSpeed(speedSet);
			motor3.setSpeed(speedSet);
			motor4.setSpeed(speedSet);
			delay(5);
		}
	}
}

void moveBackward() {
	goesForward=false;
	motor1.run(BACKWARD);
	motor2.run(BACKWARD);
	motor3.run(BACKWARD);
	motor4.run(BACKWARD);
	for (speedSet = 0; speedSet < MAX_SPEED; speedSet +=4) {
		// slowly bring the speed up to avoid loading down the batteries too quickly
		motor1.setSpeed(speedSet);
		motor2.setSpeed(speedSet);
		motor3.setSpeed(speedSet);
		motor4.setSpeed(speedSet);
		delay(5);
	}
}
