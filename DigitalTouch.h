/*
DigitalTouch.h - Library for Capacitive touch sensor with LED using only one Pin

Version 1.0.0 (17.01.2019)

This Code is adapted from the AnalogTouch library by NicoHood.
https://github.com/NicoHood/AnalogTouch
(You find more similarities in the example code than in the library code.)
The measurement method is changed to digital and the median filter has been added.
The average filter is moved into a separate function. Due to this the function names
and parameters are not compatible.

This code is for 8-Bit AVR and tested on Arduino Mega 2560.
However, the intention of the code is to run on very small controllers
like ATTINY* with small memory and less pins. I managed to limit the size
of most variables to 8 bit. Global variables are not used by the
library. In the main program, some globals are required depending on
the calibration method, see example sketch. You can also use a fixed
calibration without any global variables (not recommended).

You may have to adapt the direct port-reading in "digitalTouchRead" for other
controllers, but for most 8-Bit AVRs it should work.


-------
LICENSE
-------

Copyright (c) 2018 Rainer Urlacher
under the MIT License, see github repository


--------------------------
BACKGROUND AND DESCRIPTION
--------------------------

Functional Principles:

The touch sensing is based on measuring a capacitance change between a conductive
sensor and earth (self capacitance sensor). Every object has a small capacitive
connection to the environment and at the end to earth. The capacitance change occurs,
because the capacitance over air to conductors far away is much higher than the
capacitance between the sensor and the human body while touching. The body, as it is
also touching or is near to the ground, has a relatively good connection to earth
with some capacitance and/or resistance.

This works best, if the capacitance between the sensor and the finger is as big as
possible. So the area of the sensor should be at least slightly larger than the
fingertip, and the isolation layer should be thin. However, a very large sensor will
become an antenna that receives every noise around.

An isolation layer is required, because a DC coupling will charge or discharge
the capacitor independent from its size. The high amplidudes you can get with a
non-isolated sensor are caused by static discharge and do not lead to a stable
measurement.

The circuit must be grounded, otherwise you are measuring the combination of
two capacitors and get instable results. However, with strong filtering, a battery
usage is possible, but this becomes slow and you cannot detect the exact length of
a touch event. For battery usage I would recommend special chips that can connect
multiple sensors with only one I2C connection to the controller. More powerful
controllers have already integrated such a hardware.

It is not possible to measure the capacitance between two sensors as it is done
in advanced products like touch screens and sensor matrixes (mutual capacitance
sensor). Also for this, additional hardware is required to eliminate the effect of
the ground capacitance. Otherwise it is dominating too much. Have in mind, that
sensor design guidelines for mutual capacitance sensors, that you may find with your
own research, do not fit to our method.


Functional Details:

The measuring method is very simple and widely used in other libraries, too:
The sensor cap is discharged by a controller pin. Then the driver is set to tristate
and the capacitance is charged through a high-ohmic resistor. The level is controlled
by an input pin which has an input schmitt-trigger in all common controllers.
The time, until the pin gets HIGH is measured.

In my approach I use the same pin for discharging and measuring. Some libraries use two
pins, a send and a receive pin. This allows to measure charging and discharging a
sensor. But it is still a self capacitance sensor since it is measuring capacitance
to earth.


Connecting an LED:

The capacitor only needs to be charged up to the HIGH level of the input pin, so it
is possible to use the full Vdd level to light an LED. The forward voltage of the LED
must be higher than the input HIGH level. Otherwise the capacitor can not be charged
high enough.
You may need a blue or white LED plus an additional diode to achieve this. However,
if you have multiple sensors, you can share this extra diode between all channels.
There are also some LEDs that already have more than 3V forward voltage.
If it is beneficial in your design, you also can use two red LEDs in series. One red
LED has a forward voltage of around 1.6V, a blue LED has around 2.6V. This can differ,
best is to find out with some experiments.
Make sure you have enough margin. The HIGH level and/or the forward voltage of the diode
may change over time and over temperature.


Other Methods:

There is also the method to precharge an additional capacitor and the sensor to different
voltage levels and then connect them and measure the resulting voltage. The trick is
to use the sample-and-hold capacitor of the ADC for this, the ADC must measure the voltage
anyway. This method is introduced by Atmel in their QTouch (R) library as "QTouchADC".
The "AnalogTouch" library by NicoHood for instance, which is the basis of my code,
does exactly this. It is a very nice method, please check which fits best to your design.
The big advantage of the method is that it does not need the high-ohmic resistor.
An ESD protection is recommended, but if you make sure that the sensor is safely
isolated, you can omit this resistor, too, and get a sensor without any external
components! Of course, for the same reason you can omit the ESD resistor also in the
digital approach.
With a LED at the pin you need the resistor anyway to limit the LED current.

In my tests I did not find significant differences between the AnalogTouch library and
my DigitalTouch library regarding resolution and reliability. I even managed to connect
a LED to the sensing pin using AnalogTouch, but this was more critical than with
the digital method. It did depend on the sensor size, because the measured voltage is
depending on the relation between the two capacitors of the sensor and the ADC.

With the QTouchADC method of course you must use the ADC. So if you want to save the
power, if you have no free ADC pin available or if you don't want to multiplex the ADC
with other channels, you cannot use it. E.g. if you measure another high impedance signal
in your application, you may not want that the sample-and-hold cap in the ADC must be
charged to this signal level again and again. So it very much depends on your application
which method is preferable.

If you want to make a touch sensor from a very big component like the housing of
a device or a lamp, you must use a completely different approach. Here, it is better to
rely on the static charge instead on the capacitance. Just amplify the signal with some
transistors, clamp the result to your logic level, and connect it to your controller. Filter
the signal by hardware or software. An isolation of the sensor is counterproductive in this case.


Short application summary:

Sensor:
- sensor must be isolated (e.g. use tape)
- circuit should be grounded to earth for stable results
- sensor size around 100mm^2 work fine, larger sensors increase sensitivity, too large is noisy
- sensor should be a solid plane

Wiring:
- resistor Rs between pin and sensor: for ESD protection and optionally for current limiting of LED
- resistor Rp between Vdd and (sensor or pin): for charging up the sensor cap slowly
- LED between sensor and Vss (ground)

Rs = 1..22kOhm, with 22kOhm I got a good brightness with a blue LED for an indicator

Rp = 100k..10MOhm, with 1MOhm I got good results, higher Rp increases resolution and noise

LED forward voltage > input-pin HIGH-level
Add an extra diode between LED and Vss if required. This diode can be shared for multiple
channels.


Have fun!

*/

// Include guard
#pragma once

// Software version
#define DIGITALTOUCH_VERSION 100

// Include Arduino functions to read pins
#include "Arduino.h"

// function digitalTouchRead
// takes one sample of measurement
// please use filter methods digitalTouchAverage or digitalTouchMedian for a more reliable result
static inline uint8_t digitalTouchRead(uint8_t pin)
{
	// discharge sensor cap by driving a LOW signal
	digitalWrite(pin, LOW);
	pinMode(pin, OUTPUT);

	// Loops are faster (higher resolution) if we use direct read and get following values outside the loop.
	volatile uint8_t *inputRegister = portInputRegister(digitalPinToPort(pin));
	uint8_t inputMask = digitalPinToBitMask(pin);

	// with interrupts during the measurement we would miss a lot of counts
	noInterrupts();
	
	// switch off driver, sensor cap starts to charge through external resistor
	pinMode(pin, INPUT);
	
	// loop counter to measure the charging time, start at 1, 0 is overflow
	uint8_t cycleCounter = 1;
	
	// charge the sensor until signal is HIGH or counter is 0 (= overflow)
	// this loop must be fast in order to get a good resolution -> direct port reading is used
	// the term "(*inputRegister & inputMask)" is a non-zero byte if the input is HIGH
	// (hard-coding the values of *inputRegister and inputMask would be even faster)
	while (!(*inputRegister & inputMask) && cycleCounter) cycleCounter++;

	interrupts();

	// discharge sensor cap (or use pin for LED, see example sketch)
	pinMode(pin, OUTPUT);

	// reduce cycleCounter by 1 since it started at 1, on overflow it is zero and will become 255 now
	return cycleCounter - 1;
}


// function digitalTouchAverage
// take the average from a number of samples
// This method is useful, if you want to have a high number of samples for best results.
// It also can be used with samples = 1 (default), it will add one prior sample, which is ignored
// for better stability after the pin has been used for a LED.
uint8_t digitalTouchAverage(uint8_t pin, uint8_t samples = 1)
{
	// Measure more than once to get a more precise result
	// (even one more than specified, first will be ignored for more stability)
	uint16_t value = 0;
	for (uint8_t i = 0; i <= samples; i ++)
	{
		// Measure and add to average but ignore first sample
		if (i > 0) value += digitalTouchRead(pin);
	}

	// Return average
	return (uint8_t)(value / (uint16_t)samples);
}


// function digitalTouchMedian
// take the median from three samples
// The median is more useful for a small number of samples (3 in this case), because it ignores
// single outliers completely while the average would be strongly affected.
// It also adds one prior sample, which is ignored for stability after the pin has been used
// for a LED. 
uint8_t digitalTouchMedian(uint8_t pin)
{
	// first sample is ignored
	digitalTouchRead(pin);

	// take three samples
	uint8_t value0 = digitalTouchRead(pin);
    uint8_t value1 = digitalTouchRead(pin);
    uint8_t value2 = digitalTouchRead(pin);

	// find the median, which is the middle value in a sorted list; no value is modified
	if (value0 < value1) {
		if (value1 < value2) {
			return value1;
		} else {
			if (value0 < value2) {
				return value2;
			}
		}
	} else {
		if (value2 < value1) {
			return value1;
		} else {
			if (value2 < value0) {
				return value2;
			}
		}
	}
	return value0;
}

