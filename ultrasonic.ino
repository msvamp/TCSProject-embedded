// Library for ultrasonic sensors - https://playground.arduino.cc/Code/NewPing/
#include <NewPing.h>

NewPing sensor[2] = {null,null};

// NewPing(TRIGGER_PIN,ECHO_PIN,MAX_DISTANCE)
void _ultrasonic
(uint8_t t0, uint8_t t1, uint8_t e0, uint8_t e1) {
	// Forward sensor
	sensor[0]=NewPing(t0,e0,MAX_DIST);
	// Reverse sensor
	sensor[1]=NewPing(t1,e1,MAX_DIST);
}

// Do we have safe distance for the given sensor?
inline bool getsafe(uint8_t s) {
	return (sensor[s].convert_cm(
		sensor[s].ping_median(4)
	)>SAFE_GAP);
}