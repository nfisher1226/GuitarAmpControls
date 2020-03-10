#Guitar Amplifier Controls
Uses an Arduino to control guitar amplifier functions.

##Basic functions:
	Startup timer counts a 15 second delay after power on before switching
		on the B+ high voltage via a relay circuit
	Channel switching controls two banks of vactrols to switch between
		channels
	Four different waveforms of LFO are output simultaneously. For
		simplicity switching is handled externally in the analog realm.

##Pin connections
Mains (B+) relay circuit - digital pin 12
Channel Select - digital pin 2
Channel 1 output (vactrol/relay circuit) - digital pin 4
Channel 2 output (vactrol/relay circuit) - digital pin 5
Tremolo indicator light - digital pin 7
Tremolo speed control - analog pin A1
Sine LFO - digital pin 3
Sawtooth LFO - digital pin 9
Reverse Sawtooth LFO - digital pin 10
Square wave LFO - digital pin 11

##Usage Notes
Channel select and tremolo on/off switches should switch the pin between
5v supply (on) and ground (off).

Tremolo speed control pot is wired in reverse with bottom leg connected
to either 5v or 3.3v supply, wiper to analog pin A1 and top leg to
ground. The speed range can be adjusted by the addition of series
resistors on either side of the speed pot, to limit the voltage swing
within the pot's travel. Higher voltage = slower speed. The default
range provided is more than adequate for just about any appliocation; if
anything it will go from faster than ever needed to too slow to be useful
musically.

Care must be taken to isolate the Arduino from the voltages present in a
tyical valve based amplifier and to keep the current demands within what
the Arduino can supply. Generally speaking, the pin current should not
drive the various leds and relays directly, rather provide a seperate
power supply and use the pin signals to switch small signal transistors
on and off to drive as many leds/vactrols/relays as required.

The LFO signals are generated using PWM (pulse width modulation) and as
such are a very "dirty" signal that should not be used directly without
first using a good low pass filter to remove the high frequency noise
present. It is recommended to also use optical isolation to apply the
LFO signal. Bias modulation can still be done by means of a voltage
divider placed within a fixed bias circuit. The LFO signals all go to
their mid point when the tremolo switches off, giving a theoretical swing
in volume 50% above and below the "off" volume, to avoid the perception
of an overall increase or decrease in volume when switching the tremolo
on or off.
