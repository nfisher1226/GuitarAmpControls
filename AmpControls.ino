/* 
  * Controlled startup eliminates 'Standby' switch
  * Control two banks of vactrols for channel switching
  * Initialize lfo for tremolo and select waveform
  * 
  * Written 03/2020 by Nathan Fisher <nfisher.sr@gmail.com>
  * 
  * mains relay circuit on pin 12
  * channel switches pin 2 between 5v and ground
  * channel 1 output (relay or vactrol) pin 3
  * channel 2 output (relay or vactrol) pin 4
  * LFO A on pin 5
  * LFO B on pin 6
  * trem on/off switches pin 7 between 5v and ground
  * tremolo indicator led on pin 9
 */

// setup the pins for various purposes
// connect our mains relay circuit to pin 2
const int mainsRelayPin = 12;

// channel switching
const int chswitchPin = 2;	// channel switch spst switch on/off
const int ch1Pin = 3;		// first bank of vactrols
const int ch2Pin = 4;		// second bank of vactrols
int chswitchState = 0;		// variable reading channel switch state

// tremolo section
int trswitchState = 0;
int trmode = 0;
const int traPin = 5;		// pwm wave here
const int trbPin = 6;
const int trswitchPin = 7;	// tremolo on/off switch
const int trindPin = 9;

// sine wave positions
byte sina = 0;
byte sinb = 0;
// the value for each position in the sine and sine reverse tables
byte s = 0;
byte r = 127;
// the tremolo rate
int interval = 10;
// counts the intervals that have passed, increments each time
int loopCount = 0;
// decrements each interval
int looprevCount = 255;

// store last update time
unsigned long previousMicros = 0;

// Our sine wave table for trem
const byte sineTable[] = { 
128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167,
170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206,
208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235,
237, 238, 240, 241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252,
253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254,
254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241,
240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215,
213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179,
176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137,
134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90,
88, 85, 82, 79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42,
40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10,
9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2,
3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29,
31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73,
76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121,
124
};

// the setup function runs once when you press reset or power the board
void setup() {
	// initialize digital pin mainsRelayPin as an output.
	pinMode(mainsRelayPin, OUTPUT);
	// wait for the valves to warm up
	delay(15000);
	// switch the relay on
	digitalWrite(mainsRelayPin, HIGH);
	// initialize the vactrol pins as outputs
	pinMode(ch1Pin, OUTPUT);
	pinMode(ch2Pin, OUTPUT);
	pinMode(trindPin, OUTPUT);
	// initialize tremolo pins as output for square wave use
	pinMode(traPin, OUTPUT);
	pinMode(trbPin, OUTPUT);
}


// the program
void loop() {
	// get the channel switch state
	chswitchState = digitalRead(chswitchPin);
	
	// switch banks of vactrols on/off for channel switching
	if (chswitchState == HIGH) {
		digitalWrite(ch1Pin, HIGH);
		digitalWrite(ch2Pin, LOW);
	} else {
		digitalWrite(ch1Pin, LOW);
		digitalWrite(ch2Pin, HIGH);
	}

    unsigned long currentMicros = micros();
	// switch the tremolo on/off
	trswitchState = digitalRead(trswitchPin);
	if (trswitchState == HIGH) {
		interval = (analogRead(1) * 20 + 1);
		if (currentMicros - previousMicros >= interval) {
			// save the last pwm state change time
			previousMicros = currentMicros;
			// reset loopCount and looprevCount at the end of cycle
			if (loopCount >= 255) {
				loopCount = 0;
			}
			if (looprevCount <= 0) {
				looprevCount = 255;
			}
			// increment through the sine wave tables
			s++;
			r++;
			// increment the loop count, for square and saw waves
			loopCount++;
			// decrement the looprev count for sawrev wave
			looprevCount--;
		}
		trmode = (analogRead(2)); // read our tremolo mode switch
		if (trmode < 300) {
			sina = sineTable[s];
			sinb = sineTable[r];
			analogWrite(traPin, sina);
			analogWrite(trbPin, sinb);
			analogWrite(trindPin, sinb);
		} else if (trmode < 600) {
			analogWrite(traPin, loopCount);
			analogWrite(trbPin, looprevCount);
			analogWrite(trindPin, looprevCount);
		} else if (trmode < 900) {
			analogWrite(traPin, looprevCount);
			analogWrite(trbPin, loopCount);
			analogWrite(trindPin, loopCount);
		} else {
			if (loopCount <= 127) {
				digitalWrite(traPin, HIGH);
				digitalWrite(trbPin, LOW);
				digitalWrite(trindPin, LOW);
			} else {
				digitalWrite(traPin, LOW);
				digitalWrite(trbPin, HIGH);
				digitalWrite(trindPin, HIGH);
			}
		}
	} else {
		digitalWrite(trindPin, LOW);
		// set the LFO pins to mid level
		analogWrite(traPin, 127);
		analogWrite(trbPin, 127);
	}
}
