// Library for ultrasonic sensors - https://playground.arduino.cc/Code/NewPing
#include <NewPing.h>
NewPing sensor[2] = {null,null};

#define MAX_DIST 100
#define SAFE_GAP 30
#define SHTWTMAX 5000

unsigned long shortwaitstart=0;
enum mstate prevmotion=FWD;

// NewPing(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE)
void _ultrasonic
(uint8_t t0, uint8_t t1, uint8_t e0, uint8_t e1) {
	// Forward sensor
	sensor[0]=NewPing(t0,e0,MAX_DIST);
	// Reverse sensor
	sensor[1]=NewPing(t1,e1,MAX_DIST);
}

// Do we have safe distance for the given sensor?
bool getsafe(uint8_t s) {
	return (sensor[s].convert_cm(
		sensor[s].ping_median()
	)>SAFE_GAP);
}

inline bool is_waiting() {return (millis()-shortwaitstart)<SHTWTMAX;}

void ultracheck() {
	if((motion==FWD && !getsafe(0)) || (motion==REV && !getsafe(1))) {
		#ifdef DEBUG
			Serial.println("Obstacle detected! Motion direction is "+((motion==FWD)?"FWD":"REV"));
			Serial.println("Halting for a short interval...");
		#endif
		prevmotion=motion;
		motion=HLT;
		shortwaitstart=millis();
	}
	else if(motion==HLT && is_waiting() && getsafe(prevmotion)) {
		#ifdef DEBUG
			Serial.println("Obstacle has been cleared!");
			Serial.println("Restoring previous motion direction "+((prevmotion==FWD)?"FWD":"REV"));
		#endif
		motion=prevmotion;
	}
	else if(motion==HLT && !is_waiting()) {
		motion=(prevmotion==FWD)?REV:FWD;
		#ifdef DEBUG
			Serial.println("Short halting interval expired!");
			Serial.println("Inverting motion direction to "+((motion==FWD)?"FWD":"REV"));
		#endif
	}
}