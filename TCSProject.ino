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
#define CHECKINT 500
//#define DEBUG		// Uncomment during testing

// Motion states
enum {FWD,REV,HLT} motion;
bool is_watering=0;

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

  t.every(500,setMotion);
  t.every(60000,battstatus);
}

// Check battery status, set battery LEDs and block for critical battery
void battstatus() {
	// To-do: Write code
}

  // To-do: Attach this function to interrupt, remove too many timers
  // https://www.allaboutcircuits.com/technical-articles/using-interrupts-on-arduino/
unsigned long wstart=0,wend=0;

void blockcheck() {
  bool is_held=!digitalRead(MOT_HLD);
  unsigned long stdiff=millis()-wstart;
	
	if(is_held && !is_watering) {
    is_watering=1;
    motion=HLT;
    wstart=millis();
	}
	else if(is_watering && wtdiff<WATR_MIN) {}
  else if(is_watering && wtdiff>=WATR_MIN && wtdiff<WATR_MAX) {
    if(is_held) {
      
    }
  }
}

void loop() {
	t.handle();
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
