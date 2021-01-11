// Library for ultrasonic sensors - https://playground.arduino.cc/Code/NewPing
#include <NewPing.h>
NewPing *sensor;

// Library for queueing values
#include <cQueue.h>
Queue_t lastsafe[2];

#define MAX_DIST 400
#define SAFE_GAP 30
#define SHTWTMAX 5000
#define DSF_VAL 5
#define ZSF_VAL 3

unsigned long shortwaitstart=0;
enum mstate prevmotion=FWD;

// NewPing(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE)
void _ultrasonic
(uint8_t t0, uint8_t t1, uint8_t e0, uint8_t e1) {
	sensor=(NewPing*)calloc(2,sizeof(NewPing));
	q_init(&lastsafe[0],sizeof(uint8_t),DSF_VAL,FIFO,true);
	q_init(&lastsafe[1],sizeof(uint8_t),DSF_VAL,FIFO,true);

	for(uint8_t j=0; j<2; ++j)
		for(uint8_t i=DSF_VAL; i>0; --i)
			q_push(&lastsafe[j],0);

	// Forward sensor
	sensor[0]=NewPing(t0,e0,MAX_DIST);
	// Reverse sensor
	sensor[1]=NewPing(t1,e1,MAX_DIST);
}

// Do we have safe distance for the given sensor?
bool getsafe(uint8_t s) {
	uint8_t zerosafe=ZSF_VAL,d=0;

	while(d==0 && (zerosafe--)>0) {
		d=sensor[s].ping_cm();
		delay(1);
	}

	if(d==0) {
		#ifdef DEBUG
			Serial.print("Ultrasonic sensor ");
			Serial.print(s);
			Serial.println(" produced 3 consecutive zero values!");
		#endif
		return false;
	}

	q_push(&lastsafe[s],d);

	#ifdef DEBUG
		Serial.print("Ultrasonic sensor ");
		Serial.print(s);
		Serial.print(" :: [ ");
	#endif

	bool finals=true;
	for(uint8_t i=0,val=0; i<DSF_VAL; ++i) {
		q_peekIdx(&lastsafe[s],&val,i);

		#ifdef DEBUG
			Serial.print(val);
			Serial.print(" ");
		#endif

		if(finals && val<=SAFE_GAP)
			finals=false;
	}

	#ifdef DEBUG
		Serial.print("] :: ");
		Serial.println(finals?"true":"false");
	#endif

	return finals;
}

inline bool is_waiting() {return (millis()-shortwaitstart)<SHTWTMAX;}

void ultracheck() {
	if((motion==FWD && !getsafe(0)) || (motion==REV && !getsafe(1))) {
		#ifdef DEBUG
			Serial.print("Obstacle detected! Motion direction is ");
			Serial.println((motion==FWD)?"FWD":"REV");
			Serial.println("Halting for a short interval...");
		#endif
		prevmotion=motion;
		motion=HLT;
		shortwaitstart=millis();
	}
	else if(motion==HLT && is_waiting() && getsafe(prevmotion)) {
		#ifdef DEBUG
			Serial.println("Obstacle has been cleared!");
			Serial.print("Restoring previous motion direction ");
			Serial.println((prevmotion==FWD)?"FWD":"REV");
		#endif
		motion=prevmotion;
	}
	else if(motion==HLT && !is_waiting()) {
		motion=(prevmotion==FWD)?REV:FWD;
		#ifdef DEBUG
			Serial.println("Short halting interval expired!");
			Serial.print("Inverting motion direction to ");
			Serial.println((motion==FWD)?"FWD":"REV");
		#endif
	}
}
