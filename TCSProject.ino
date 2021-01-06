/* TCS Agricultural Project
 * TY B. Tech. CSE
 * MIT-WPU 2020-21
 */

#include <Timer.h>
Timer t;

// Motion states
enum mstate {FWD,REV,HLT} motion;

#define UCHECK_INT 500
#define PCHECK_INT 10000
#define DEBUG		// Uncomment during testing

/* For the Nano, we have these pins available -
 * Digital: D2 to D13	(05 to 16)
 * Analog:	A7 to A0	(19 to 26)
 */

bool powerhold=false;

const int
	MOT_HLD=5,
	SOL_CTL=16,
	BAT_PIN=A6,	CHG_PIN=A7
;

// Other code files
#include "ultrasonic.ino"
#include "watering.ino"
#include "motorcode.ino"

void setup() {
	#ifdef DEBUG
		Serial.begin(115200);
		Serial.println("=== POWER ON ===");
		Serial.println("Setting up pins...");
	#endif

	// Ultrasonic control
	_ultrasonic(7,11,8,12);

	// ESP32 control
	digitalWrite(MOT_HLD,1);
	pinMode(MOT_HLD,0);

	// Pump control
	pinMode(SOL_CTL,1);

	// Power source control
	pinMode(BAT_PIN,0);
	pinMode(CHG_PIN,0);

	// Motor control
	pinMode(RMOT_F,1);	pinMode(RMOT_R,1);
	pinMode(FMOT_F,1);	pinMode(FMOT_R,1);

	// Startup delay
	#ifdef DEBUG
		Serial.println("Startup delay...");
	#endif
	delay(5000);

	#ifdef DEBUG
		Serial.println("Starting timed tasks...");
	#endif

	int
		uchecker=t.every(UCHECK_INT,ultracheck),
		bchecker=t.every(BCHECK_INT,blockcheck),
		tservice=t.every(MSERVICE_INT,motorstep)
	;

	// These tasks should never stop
	batstats=t.every(PCHECK_INT,battstatus);
}

// Check battery status, set battery LEDs and block for critical battery
void battstatus() {
	float x=100*analogRead(BAT_PIN)/1024.0;
	#ifdef DEBUG
		Serial.print("Battery voltage indicator is at ");
		Serial.println(x);
	#endif

	// Stop motion if battery is low
	if(x<50) {
		motorstop();
		digitalWrite(SOL_CTL,0);
		t.stop(uchecker);
		t.stop(bchecker);
		powerhold=true;
		delay(PCHECK_INT-1);
	}
	else if(powerhold) {
		uchecker=t.every(UCHECK_INT,ultracheck);
		bchecker=t.every(BCHECK_INT,blockcheck);
		powerhold=false;
	}
}

#include "watering.ino"

void loop() {
	t.update();
	if(motion==FWD)
		m_dirn=1;
	else if(motion==HLT)
		mspeed=m_dirn=0;	// Redundancy is cool
	else
		m_dirn=-1;
}