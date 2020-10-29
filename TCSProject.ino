/* TCS Agricultural Project
 * TY B. Tech. CSE
 * MIT-WPU 2020-21
 */

// Library for ultrasonic sensors - https://playground.arduino.cc/Code/NewPing/
#include <NewPing.h>

#define MAX_DIST 100
#define SAFE_GAP 30
#define WATR_MIN 1000
#define WATR_MAX 3000
#define INERTINT 1000
//#define DEBUG		// Uncomment during testing

/* For the Nano, we have these pins available -
 * Digital: D2 to D13 (05 to 16)
 * Analog:	A7 to A0	(19 to 26)
 */

NewPing sensor[2] = {
	// NewPing(TRIGGER_PIN,ECHO_PIN,MAX_DISTANCE)
	NewPing(5,7,MAX_DIST),	// Forward sensor
	NewPing(6,8,MAX_DIST)	// Reverse sensor
};

const int
	FWD_LED=9,
	REV_LED=10,
	MOT_HLD=11,
	BAT_PIN=A7,
	SOL_CTL=12,
	LOW_BAT=13,
	CRI_BAT=14
;

bool gl_en=0;				// Global motion on/off switch

// BATTERY STATUS
int value;// INPUT FROM BATTERY
int Red = 8;// RED LED TO PIN 8

void setup() {
	
	pinMode(8, OUTPUT);//LED OUTPUT TO INDICATE BATTERY LOW
	
	#ifdef DEBUG
		Serial.begin(115200);
		Serial.println("POWER ON\n");
	#endif

	// Indcators
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
  delay(100);
  value = analogRead(0);
  if (value<3.74)
  {
    digitalWrite(Red, HIGH);
  }
  else
  {
    digitalWrite(Red, LOW);
  }
  delay(500);
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

// Do we have safe distance for the given sensor?
inline bool getsafe(size_t s=0) {
	return (sensor[s].convert_cm(
		sensor[s].ping_median(4)
	)>SAFE_GAP);
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
