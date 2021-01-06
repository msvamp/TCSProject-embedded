#define WATR_MIN 1000
#define WATR_MAX 3000
#define BCHECK_INT 10

// Will forcefully stop the pump when true
bool wforcestopped;

// Timed function to set the force stopper
void waterforcestop() {wforcestopped=true;}

void blockcheck() {
	if(digitalRead(MOT_HLD) || motion==REV) return;

	#ifdef DEBUG
		Serial.println("Motion hold encountered!");
		Serial.println("Stopping motion, starting pump...");
	#endif

	// Start pump, stop motion
	digitalWrite(SOL_CTL,1);
	motion=HLT;

	// Stop motion-related timed tasks
	t.stop(uchecker);
	t.stop(bchecker);
	wforcestopped=false;

	// Set timers for min and max intervals
	t.after(WATR_MIN,startwaterstopper);
	t.after(WATR_MAX,waterforcestop);
}

void startwaterstopper() {
	#ifdef DEBUG
		Serial.println("Minimum watering interval completed!");
		Serial.println("Waiting for ESP pin to go off...");
	#endif
	t.stop(bchecker);		// Redundancy is cool
	bchecker=t.every(BCHECK_INT,waterstopper);
}

void waterstopper() {
	if(!wforcestopped && !digitalRead(MOT_HLD) && !powerhold) return;

	#ifdef DEBUG
		Serial.println("Pump is now stopping! Motion resuming in FWD direction!");
		Serial.println("\tTimeout Expired: "+(wforcestopped?"1":"0"));
		Serial.println("\tESP hold released: "+(digitalRead(MOT_HLD)?"1":"0"));
		Serial.println("\tPower hold: "+(powerhold?"1":"0"));
	#endif

	// Reset motion and pump
	motion=HLT;		// Let 'ultracheck' deal with motion while resuming
	digitalWrite(SOL_CTL,0);

	// Restore natural behaviour for 'ultracheck'
	prevmotion=FWD;
	shortwaitstart=millis();

	// Restart original timed tasks
	uchecker=t.every(UCHECK_INT,ultracheck);
	t.stop(bchecker);
	bchecker=t.every(BCHECK_INT,blockcheck);
}