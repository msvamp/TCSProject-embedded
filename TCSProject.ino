/* TCS Agricultural Project
 * TY B. Tech. CSE
 * MIT-WPU 2020-21
 */

#include <Timer.h>
Timer t;

// Motion states
enum mstate {FWD,REV,HLT} motion;

#define UCHECK_INT 100
#define PCHECK_INT 30000
#define CCHECK_INT 500
#define CCHECK_ACT 5000
//#define DEBUG		// Uncomment during testing

/* For the Nano, we have these pins available -
 * Digital: D2 to D13	(05 to 16)
 * Analog:	A7 to A0	(19 to 26)
 */

bool powerhold=false,chghold=false;

const int
	MOT_HLD=2,
	SOL_CTL=LED_BUILTIN,
	PST_PIN=3,
	BAT_PIN=A6,	CHG_PIN=A7,
	RMOT_F=5, RMOT_R=6,
	FMOT_F=9,	FMOT_R=10,
	LEDG=A0,/*LEDR1=A1,*/LEDR2=A2
;

int uchecker,bchecker,tservice,batstats,chgstats;

// Other code files
#include "motorcode.h"
#include "watering.h"
#include "ultrasonic.h"

void setup() {
	#ifdef DEBUG
		Serial.begin(115200);
		Serial.println("=== POWER ON ===");
		Serial.println("Setting up pins...");
	#endif

	// Ultrasonic control
	_ultrasonic(7,11,8,12);

	// ESP32 control
	pinMode(MOT_HLD,INPUT_PULLUP);
	digitalWrite(MOT_HLD,1);

	// Pump control
	pinMode(SOL_CTL,1);
	digitalWrite(SOL_CTL,0);

	// Power control
	pinMode(BAT_PIN,0);
	pinMode(CHG_PIN,0);
	pinMode(PST_PIN,1);
	digitalWrite(PST_PIN,0);

	// Motor control
	pinMode(RMOT_F,1);	pinMode(RMOT_R,1);
	pinMode(FMOT_F,1);	pinMode(FMOT_R,1);
	motorstop();

	// Status LEDs
	pinMode(LEDG,1);	digitalWrite(LEDG,1);
	//pinMode(LEDR1,1); digitalWrite(LEDR1,1);	// Dead LED
	pinMode(LEDR2,1); digitalWrite(LEDR2,1);

	// Startup delay
	#ifdef DEBUG
		Serial.println("Startup delay...");
	#endif
	delay(5000);

	#ifdef DEBUG
		Serial.println("Starting timed tasks...");
	#endif

	motion=FWD;
	uchecker=t.every(UCHECK_INT,ultracheck,0);
	bchecker=t.every(BCHECK_INT,blockcheck,0);
	tservice=t.every(MSERVICE_INT,motorstep,0);

	// Power-related tasks
	batstats=t.every(PCHECK_INT,battstatus,0);
	chgstats=t.every(CCHECK_INT,chgstatus,0);
}

void chgstatus() {
	if(!chghold && analogRead(CHG_PIN)>100) {
		#ifdef DEBUG
			Serial.println("Charging is active!");
		#endif

		t.stop(uchecker);
		t.stop(bchecker);
		digitalWrite(SOL_CTL,0);
		motorstop();
		chghold=true;

		t.stop(chgstats);
		chgstats=t.every(CCHECK_ACT,chgstatus,0);
	}
	else if(chghold && analogRead(CHG_PIN)<50) {
		t.stop(chgstats);
		chgstats=t.every(CCHECK_INT,chgstatus,0);
		uchecker=t.every(UCHECK_INT,ultracheck,0);
		bchecker=t.every(BCHECK_INT,blockcheck,0);
		chghold=false;
	}
}

// Check battery status, set battery LEDs and block for critical battery
void battstatus() {
	if(chghold) {
		#ifdef DEBUG
			Serial.println("Charging is active, ignoring battery check");
		#endif
		return;
	}

	uint16_t x=analogRead(BAT_PIN);
	#ifdef DEBUG
		Serial.print("Battery voltage indicator is at ");
		Serial.println(x);
	#endif

	// Stop motion if battery is low
	if(x<536) {
		motorstop();
		digitalWrite(SOL_CTL,0);
		t.stop(uchecker);
		t.stop(bchecker);
		powerhold=true;
		//delay(PCHECK_INT-1);	// Extra safety
	}
	else if(powerhold && x>621) {
		uchecker=t.every(UCHECK_INT,ultracheck,0);
		bchecker=t.every(BCHECK_INT,blockcheck,0);
		powerhold=false;
	}
}

void loop() {
	t.update();

	if(motion==FWD) {
		digitalWrite(LEDG,1);
		m_dirn=1;
	}
	else if(motion==HLT) {
		digitalWrite(LEDG,0);
		mspeed=m_dirn=0;	// Redundancy is cool
	}
	else if(motion==REV) {
		digitalWrite(LEDG,1);
		m_dirn=-1;
	}

	if(powerhold || chghold)
		digitalWrite(PST_PIN,1);
	else
		digitalWrite(PST_PIN,0);
}
