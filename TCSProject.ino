/* TCS Agricultural Project
 * TY B. Tech. CSE
 * MIT-WPU 2020-21
 */

#include <Event.h>
#include <Timer.h>
Timer t;

#define MAX_DIST 100
#define SAFE_GAP 30
#include "ultrasonic.ino"

#define WATR_MIN 1000
#define WATR_MAX 3000
#define INERTINT 1000
//#define DEBUG		// Uncomment during testing

// Motion states
enum {FWD,REV,HLT} motion;

/* For the Nano, we have these pins available -
 * Digital: D2 to D13	(05 to 16)
 * Analog:	A7 to A0	(19 to 26)
 */

const int
	FWD_LED=9,
	REV_LED=10,
	MOT_HLD=11,
	BAT_PIN=A7,
	SOL_CTL=12,
	LOW_BAT=13,
	CRI_BAT=14
;

// Ultrasonic uses pins 5-8
_ultrasonic(5,6,7,8);

void setup() {
	#ifdef DEBUG
		Serial.begin(115200);
		Serial.println("POWER ON\n");
	#endif

	// Indicators
	pinMode(FWD_LED,1);
	pinMode(REV_LED,1);

	// Actual outputs
	// To-do: add pins for motor outputs
	pinMode(SOL_CTL,1);

	// Battery-related pins
	pinMode(BAT_PIN,0);
	digitalWrite(BAT_PIN,0);
	pinMode(LOW_BAT,1);
	pinMode(CRI_BAT,1);

	// ESP pin
	pinMode(MOT_HLD,0);
	digitalWrite(MOT_HLD,1);

	// Startup delay
	delay(10000);
}

// Check battery status, set battery LEDs and block for critical battery
void battstatus() {
	// To-do: Write code
}

// Check if time interval is in range
inline bool isinert(unsigned long t,unsigned long i=INERTINT)
{return (millis()-t)<i;}

unsigned long wstart=0,wend=0;
bool is_watering=0;
// If signal received from ESP, block movement, enable water solenoid
void blockcheck() {
	if(isinert(wend)) return;
	bool is_held=!digitalRead(MOT_HLD);
	// To-do: Attach this function to interrupt, remove too many timers
	// https://www.allaboutcircuits.com/technical-articles/using-interrupts-on-arduino/
	if(is_held && !is_watering) {
		gl_en=0;
		digitalWrite(SOL_CTL,1);
		is_watering=wstart=millis();
	}
	else if(!isinert(wstart,WATR_MAX) || !is_held && !isinert(wstart,WATR_MIN)) {
		gl_en=1;
		digitalWrite(SOL_CTL,0);
		is_watering=false;
		wend=millis();
	}
}

unsigned long gnint=0;
void go_(bool dir=0) {
	if(isinert(gnint)) return;

	if(dir) {	// To move reverse
		digitalWrite(FWD_LED,0);
		digitalWrite(REV_LED,1);
	}
	else {		// To move forward
		digitalWrite(FWD_LED,1);
		digitalWrite(REV_LED,0);
	}
	// To-do: Write motor driving code
}

void go_nowhere() {
	digitalWrite(FWD_LED,0);
	digitalWrite(REV_LED,0);
	gnint=millis();
	// To-do: Write motor stopping code
}

void loop() {
	battstatus();
	blockcheck();
	motcheck();
	if(gl_en) {
		// To-do: refine motion logic and prevent sudden movements
		// i.e. separate into setmotion() and applymotion()

		// Check forward sensor
		if(getsafe())
			go_();
		// Check reverse sensor
		else if(getsafe(1))
			go_(1);
		// We are blocked, halt
		else
			go_nowhere();
	}
	else
		go_nowhere();
}

void setMotion() {
	// To-do: Increase speed gradually without blocking
	if(motion==FWD)
		moveForward();
	else if(motion==REV)
		moveBackward();
	else
		moveStop();
}
