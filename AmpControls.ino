/* 
  * Controlled startup eliminates 'Standby' switch
  * Control two banks of vactrols for channel switching
  * Initialize lfo for tremolo and select waveform
  * 
  * Written 03/2020 by Nathan Fisher <nfisher.sr@gmail.com>
  * 
  * mains relay circuit on pin 12
  * channel switches pin 2 between 5v and ground
  * trem on/off switches pin 6 between 5v and ground
  * channel 1 output (relay or vactrol) pin 4
  * channel 2 output (relay or vactrol) pin 4
  * sine wave output on pin 3
  * sawtooth output on pin 9
  * saw reverse output on pin 10
  * square wave output on pin 11
  * tremolo indicator led on pin 7
 */

// setup the pins for various purposes
// connect our mains relay circuit to pin 2
const int mainsRelayPin = 12;

// channel switching
const int chswitchPin = 2;	// channel switch spst switch on/off
const int ch1Pin = 4;		// first bank of vactrols
const int ch2Pin = 5;		// second bank of vactrols
int chswitchState = 0;		// variable reading channel switch state

// tremolo section
const int trswitchPin = 6;	// tremolo on/off switch
const int trledPin = 7;
int trswitchState = 0;
const int sinePin = 3;		// pwm sine wave here
const int sawPin = 9;		// sawtooth wave
const int sawrevPin = 10;	// reverse sawtooth wave
const int squarePin = 11;	// square wave

// sine wave position
byte position = 0;
// the value for each position in the sine table
byte s = 0;
// the tremolo rate
int interval = 10;
// counts the intervals that have passed, increments each time
int loopCount = 0;
// decrements each interval
int looprevCount = 255;

// store last update time
unsigned long previousMicros = 0;

// Our sine wave table for trem, courtesy Collin Cunningham / Makezine.com

const byte sineTable[] = { 
0x80, 0x83, 0x86, 0x89, 0x8D, 0x90, 0x93, 0x96, 0x99, 0x9C, 0x9F, 0xA2,
0xA5, 0xA8, 0xAB, 0xAE, 0xB1, 0xB4, 0xB7, 0xBA, 0xBD, 0xBF, 0xC2, 0xC5,
0xC7, 0xCA, 0xCD, 0xCF, 0xD1, 0xD4, 0xD6, 0xD9, 0xDB, 0xDD, 0xDF, 0xE1,
0xE3, 0xE5, 0xE7, 0xE9, 0xEB, 0xEC, 0xEE, 0xF0, 0xF1, 0xF3, 0xF4, 0xF5,
0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFC, 0xFD, 0xFE, 0xFE, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE,
0xFD, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6, 0xF5, 0xF3, 0xF2,
0xF0, 0xEF, 0xED, 0xEC, 0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
0xDA, 0xD7, 0xD5, 0xD3, 0xD0, 0xCE, 0xCB, 0xC9, 0xC6, 0xC3, 0xC1, 0xBE,
0xBB, 0xB8, 0xB5, 0xB3, 0xB0, 0xAD, 0xAA, 0xA7, 0xA4, 0xA1, 0x9E, 0x9B,
0x98, 0x94, 0x91, 0x8E, 0x8B, 0x88, 0x85, 0x82, 0x7E, 0x7B, 0x78, 0x75,
0x72, 0x6F, 0x6C, 0x69, 0x66, 0x63, 0x5F, 0x5C, 0x59, 0x56, 0x53, 0x50,
0x4D, 0x4A, 0x47, 0x44, 0x42, 0x3F, 0x3C, 0x39, 0x37, 0x35, 0x32, 0x30,
0x2D, 0x2B, 0x29, 0x26, 0x24, 0x22, 0x20, 0x1E, 0x1C, 0x1A, 0x18, 0x16,
0x14, 0x13, 0x11, 0x10, 0x0E, 0x0D, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06,
0x05, 0x04, 0x03, 0x03, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05,
0x06, 0x07, 0x08, 0x09, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x12, 0x14, 0x15,
0x17, 0x19, 0x1B, 0x1D, 0x1E, 0x21, 0x23, 0x25, 0x28, 0x2A, 0x2C, 0x2F,
0x31, 0x33, 0x36, 0x39, 0x3B, 0x3E, 0x41, 0x43, 0x46, 0x49, 0x4C, 0x4F,
0x51, 0x55, 0x58, 0x5B, 0x5E, 0x61, 0x64, 0x67, 0x6A, 0x6E, 0x70, 0x73,
0x77, 0x7A, 0x7D, 0x80
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
	pinMode(trledPin, OUTPUT);
	// initialize square wave pin as output
	pinMode(squarePin, OUTPUT);
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
	
	// switch the tremolo on/off
	trswitchState = digitalRead(trswitchPin);
	if (trswitchState == HIGH) {
		digitalWrite(trledPin, HIGH);
		interval = (analogRead(1) * 20 + 1);
		position = sineTable[s];
		analogWrite(sinePin, position);
		analogWrite(sawPin, loopCount);
		analogWrite(sawrevPin, looprevCount);
    unsigned long currentMicros = micros();
		if (currentMicros - previousMicros >= interval) {
			// save the last pwm state change time
			previousMicros = currentMicros;
			if (loopCount <= 127) {
				digitalWrite(squarePin, HIGH);
			} else {
				digitalWrite(squarePin, LOW);
			}
			// reset loopCount and looprevCount at the end of cycle
			if (loopCount >= 255) {
				loopCount = 0;
			}
			if (looprevCount <= 0) {
				looprevCount = 255;
			}
			// increment through the sine wave table
			s++;
			// increment the loop count, for square and saw waves
			loopCount++;
			// decrement the looprev count for sawrev wave
			looprevCount--;
		}
	} else {
		digitalWrite(trledPin, LOW);
		// set all of the LFO pins to mid level
		analogWrite(sinePin, 127);
		analogWrite(sawPin, 127);
		analogWrite(sawrevPin, 127);
		analogWrite(squarePin, 127);
	}
}
