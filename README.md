Uses an Arduino to control guitar amplifier functions.

Basic functions:
	Startup timer counts a 15 second delay after power on before switching
		on the B+ high voltage via a relay circuit
	Channel switching controls two banks of vactrols to switch between
		channels
	Four different waveforms of LFO are output simultaneously. For
		simplicity switching is handled externally in the analog realm.

Care must be taken to isolate the Arduino from the voltages present in a
tyical valve based amplifier and to keep the current demands within what
the Arduino can supply. Generally speaking, the pin current should not
drive the various leds and relays directly, rather provide a seperate
power supply and use the pin signals to switch small signal transistors
on and off to drive as many leds/vactrols/relays as required.

The LFO signals are generated using PWM (pulse width modulation) and as
such are a very "dirty" signal that should not be used directly without
first using a good low pass filter to remove the high frequency noise
present. It is recommended to also use optical isolation to apply the LFO.
