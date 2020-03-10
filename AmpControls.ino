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

int interval = 10;
byte brightness = 0;
byte a = 0;
byte b = 0;
byte c = 0;
byte d = 0;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMicros = 0;        // will store last update time

// Our wave tables for trem, sine table courtesy Collin Cunningham / Makezine.com

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

const byte sawTable[] = {
255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241,
240, 239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 226,
225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211,
210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196,
195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181,
180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166,
165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151,
150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136,
135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121,
120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106,
105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89,
88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71,
70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53,
52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35,
34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte sawrevTable[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152,
153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212,
213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227,
228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

const byte squareTable[] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
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
	
	// switch the tremolo vactol on/off
  trswitchState = digitalRead(trswitchPin);
	if (trswitchState == HIGH) {
		digitalWrite(trledPin, HIGH);
		interval = (analogRead(1) * 20 + 1);
		brightness = sineTable[a];
		analogWrite(sinePin, brightness);
		brightness = sawTable[b];
		analogWrite(sawPin, brightness);
		brightness = sawrevTable[c];
		analogWrite(sawrevPin, brightness);
		brightness = squareTable[d];
		analogWrite(squarePin, brightness);
    unsigned long currentMicros = micros();
		if (currentMicros - previousMicros >= interval) {
			// save the last pwm state change time
			previousMicros = currentMicros;
			// increment through the wave tables
			a++;
			b++;
			c++;
			d++;
		}
	} else {
		digitalWrite(trledPin, LOW);
		analogWrite(sinePin, 127);
		analogWrite(sawPin, 127);
		analogWrite(sawrevPin, 127);
		analogWrite(squarePin, 127);
	}
}
