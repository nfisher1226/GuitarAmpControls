# Guitar Amplifier Controls
Uses an Arduino to control guitar amplifier functions.

## Basic functions:
* Startup timer counts a 15 second delay after power on before switching
on the B+ high voltage via a relay circuit
* Channel switching controls two banks of vactrols to switch between
channels
* A tremolo LFO is initiated, switchable between four different wave forms

## Pin connections
* Mains (B+) relay circuit - digital pin 12
* Channel Select - digital pin 2
* Channel 1 output (vactrol/relay circuit) - digital pin 3
* Channel 2 output (vactrol/relay circuit) - digital pin 4
* LFO-A - digital pin 5
* LFO-B - digital pin 6
* Tremolo on/off switch - digital pin 7
* Tremolo indicator light - digital pin 9
* Tremolo speed control - analog pin A1
* Tremolo mode rotary switch - analog pin A2

## Usage Notes
Channel select and tremolo on/off switches should switch the pin between
5v supply (on) and ground (off).

Tremolo speed control pot is wired in reverse with bottom leg connected
to either 5v or 3.3v supply, wiper to analog pin A1 and top leg to
ground. The speed range can be adjusted by the addition of series
resistors on either side of the speed pot, to limit the voltage swing
within the pot's travel. Higher voltage = slower speed. The default
range provided is more than adequate for just about any application; if
anything it will go from faster than ever needed to too slow to be useful
musically.

Wire the four position rotary switch with equal value resistors between
each pole. Pole one connects to ground, pole 4 to 5v, common to analog
pin A2.

The tremolo LFO is output on two pins in anti-phase to each other, ie
whatever LFO-A is doing LFO-B is doing the reverse. This gives a very
flexible arrangement for panning or for controlling output tube bias
via a dual vactrol voltage divider in the bias supply circuit.

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
