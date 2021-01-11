#define MAX_SPEED 100	// Range: 0-255
#define ACC_TIME 3000

const uint16_t MSERVICE_INT=(uint16_t)(ACC_TIME/MAX_SPEED);

int8_t m_dirn=0;
uint8_t mspeed=0;

void motorstep() {
	if(m_dirn==0) {
		analogWrite(RMOT_F,0);
		analogWrite(RMOT_R,0);
		analogWrite(FMOT_F,0);
		analogWrite(FMOT_R,0);
		mspeed=0;
		return;
	}

	if(mspeed+1>MAX_SPEED)
		return;
	else
		mspeed+=1;

	if(m_dirn>0) {
		analogWrite(RMOT_F,0);
		analogWrite(RMOT_R,mspeed);
		analogWrite(FMOT_F,0);
		analogWrite(FMOT_R,mspeed);
	}
	else {
		analogWrite(RMOT_F,mspeed);
		analogWrite(RMOT_R,0);
		analogWrite(FMOT_F,mspeed);
		analogWrite(FMOT_R,0);
	}
}

void motorstop() {
	analogWrite(RMOT_F,0);
	analogWrite(RMOT_R,0);
	analogWrite(FMOT_F,0);
	analogWrite(FMOT_R,0);
	motion=HLT;
	m_dirn=mspeed=0;
}
