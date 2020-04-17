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
  * trem on/off switches pin 7 between 5v and ground
  * tremolo indicator led on pin 9
  * tremolo provided on two MCP4725 breakout cards via i2c interface
 */
 
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include "SineWave12bit.h"

Adafruit_MCP4725 dac0;
Adafruit_MCP4725 dac1;

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
const int trswitchPin = 7;	// tremolo on/off switch
const int trindPin = 9;

// sine wave positions
byte sina = 0;
byte sinb = 0;
// the value for each position in the sine and sine reverse tables
byte s = 0;
byte r = 2047;
// the tremolo rate
int interval = 0;
// counts the intervals that have passed, increments each time
int loopCount = 0;
// decrements each interval
int looprevCount = 4095;

// store last update time
unsigned long previousMicros = 0;

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	Serial.println("Starting up Amplifier Controls.");
	// initialize digital pin mainsRelayPin as an output.
	pinMode(mainsRelayPin, OUTPUT);
	// wait for the valves to warm up
	Serial.println("Warming up valves...");
	delay(15000);
	// switch the relay on
	Serial.println("Switching on mains relay.");
	digitalWrite(mainsRelayPin, HIGH);
	// initialize the vactrol pins as outputs
	pinMode(ch1Pin, OUTPUT);
	pinMode(ch2Pin, OUTPUT);
	pinMode(trindPin, OUTPUT);
	// initialize the two dac boards
	dac0.begin(0x62);
	dac1.begin(0x63);
}


// the program
void loop() {
	// get the channel switch state
	chswitchState = digitalRead(chswitchPin);
	
	// switch banks of vactrols on/off for channel switching
	if (chswitchState == HIGH) {
		Serial.println("Channel one active.");
		digitalWrite(ch1Pin, HIGH);
		digitalWrite(ch2Pin, LOW);
	} else {
		Serial.println("Channel two active.");
		digitalWrite(ch1Pin, LOW);
		digitalWrite(ch2Pin, HIGH);
	}

    unsigned long currentMicros = micros();
	// switch the tremolo on/off
	trswitchState = digitalRead(trswitchPin);
	if (trswitchState == HIGH) {
		Serial.println("Tremolo active.");
		interval = (analogRead(1) * 2 + 1);
		if (currentMicros - previousMicros >= interval) {
			// save the last pwm state change time
			previousMicros = currentMicros;
			// reset loopCount and looprevCount at the end of cycle
			if (loopCount >= 4095) {
				loopCount = 0;
			}
			if (looprevCount <= 0) {
				looprevCount = 4095;
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
			Serial.println("Outputting sine wave.");
			sina = sineTable[s];
			sinb = sineTable[r];
			dac0.setVoltage(sina, false);
			dac1.setVoltage(sinb, false);
			analogWrite(trindPin, sinb / 16);
		} else if (trmode < 600) {
			Serial.println("Outputting sawtooth wave.");
			dac0.setVoltage(loopCount, false);
			dac1.setVoltage(looprevCount, false);
			analogWrite(trindPin, looprevCount);
		} else if (trmode < 900) {
			Serial.println("Outputting sawtooth reverse wave.");
			dac0.setVoltage(looprevCount, false);
			dac1.setVoltage(loopCount, false);
			analogWrite(trindPin, loopCount);
		} else {
			Serial.println("Outputting square wave.");
			if (loopCount <= 2047) {
				dac0.setVoltage(4095, false);
				dac1.setVoltage(0, false);
				digitalWrite(trindPin, LOW);
			} else {
				dac0.setVoltage(0, false);
				dac1.setVoltage(4095, false);
				digitalWrite(trindPin, HIGH);
			}
		}
	} else {
		Serial.println("Tremolo is off");
		// set both dacs to mid level
		dac0.setVoltage(2047, false);
		dac1.setVoltage(2047, false);
		digitalWrite(trindPin, LOW);
	}
}
