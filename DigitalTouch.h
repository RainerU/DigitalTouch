/*
DigitalTouch.h - Library for Capacitive touch sensor with LED using only one Pin

Version 1.1.0 (21.01.2019)

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
library. In the main program, some globals are required for calibration.

-------
LICENSE
-------

Copyright (c) 2018 Rainer Urlacher
published under the MIT License, see github repository

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

-----------------------
SOFTWARE IMPLEMENTATION
-----------------------

Please read the extensive comments in the code of the library and the example!
There is no better way to explain the software.

I spent a huge effort to make the measuring loops (that measure the charging time of the
sensors) as fast as possible in order to get a good resolution. To use this is optional.
It requires to add some #define statements for each used sensor in the main program.

In addition, I extended the code in a way that avoids any port access through the Arduino
core functions like digitalWrite and digitalRead. This is also optional. Avoiding these
funtions removes a significant part of the Arduino core from the program memory.

The structure I found allows to do this without a lot of overhead in the compiled code.
It only appears what is really used. But the library looks complicated since the options
are controlled by a bunch of #ifdef statements. Most lines are just repetition...
If you want to understand how the library generates code, please read the comments between
the code lines in the library and in the example sketch.


Have fun!

*/

// Include guard
#pragma once

// Software version
#define DIGITALTOUCH_VERSION 110


#ifdef sensor1_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor1)
	// define "sensor1_read" in main program to enable this function, see example
	// define also "sensor1_read/input/output/low/high" for ALL sensors in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_1() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor1_low
			sensor1_low;
		#else
			digitalWrite(sensor1, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor1_input
			sensor1_input;
		#else
			pinMode(sensor1, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor1_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor1_output
			sensor1_output;
		#else
			pinMode(sensor1, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor2_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor2)
	// define "sensor2_read" in main program to enable this function, see example
	// define also "sensor2_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_2() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor2_low
			sensor2_low;
		#else
			digitalWrite(sensor2, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor2_input
			sensor2_input;
		#else
			pinMode(sensor2, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor2_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor2_output
			sensor2_output;
		#else
			pinMode(sensor2, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor3_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor3)
	// define "sensor3_read" in main program to enable this function, see example
	// define also "sensor3_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_3() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor3_low
			sensor3_low;
		#else
			digitalWrite(sensor3, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor3_input
			sensor3_input;
		#else
			pinMode(sensor3, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor3_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor3_output
			sensor3_output;
		#else
			pinMode(sensor3, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor4_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor4)
	// define "sensor4_read" in main program to enable this function, see example
	// define also "sensor4_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_4() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor4_low
			sensor4_low;
		#else
			digitalWrite(sensor4, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor4_input
			sensor4_input;
		#else
			pinMode(sensor4, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor4_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor4_output
			sensor4_output;
		#else
			pinMode(sensor4, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor5_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor5)
	// define "sensor5_read" in main program to enable this function, see example
	// define also "sensor5_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_5() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor5_low
			sensor5_low;
		#else
			digitalWrite(sensor5, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor5_input
			sensor5_input;
		#else
			pinMode(sensor5, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor5_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor5_output
			sensor5_output;
		#else
			pinMode(sensor5, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor6_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor6)
	// define "sensor6_read" in main program to enable this function, see example
	// define also "sensor6_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_6() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor6_low
			sensor6_low;
		#else
			digitalWrite(sensor6, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor6_input
			sensor6_input;
		#else
			pinMode(sensor6, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor6_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor6_output
			sensor6_output;
		#else
			pinMode(sensor6, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor7_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor7)
	// define "sensor7_read" in main program to enable this function, see example
	// define also "sensor7_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_7() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor7_low
			sensor7_low;
		#else
			digitalWrite(sensor7, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor7_input
			sensor7_input;
		#else
			pinMode(sensor7, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor7_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor7_output
			sensor7_output;
		#else
			pinMode(sensor7, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor8_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor8)
	// define "sensor8_read" in main program to enable this function, see example
	// define also "sensor8_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_8() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor8_low
			sensor8_low;
		#else
			digitalWrite(sensor8, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor8_input
			sensor8_input;
		#else
			pinMode(sensor8, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor8_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor8_output
			sensor8_output;
		#else
			pinMode(sensor8, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor9_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor9)
	// define "sensor9_read" in main program to enable this function, see example
	// define also "sensor9_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_9() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor9_low
			sensor9_low;
		#else
			digitalWrite(sensor9, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor9_input
			sensor9_input;
		#else
			pinMode(sensor9, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor9_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor9_output
			sensor9_output;
		#else
			pinMode(sensor9, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor10_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor10)
	// define "sensor10_read" in main program to enable this function, see example
	// define also "sensor10_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_10() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor10_low
			sensor10_low;
		#else
			digitalWrite(sensor10, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor10_input
			sensor10_input;
		#else
			pinMode(sensor10, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor10_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor10_output
			sensor10_output;
		#else
			pinMode(sensor10, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor11_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor11)
	// define "sensor11_read" in main program to enable this function, see example
	// define also "sensor11_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_11() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor11_low
			sensor11_low;
		#else
			digitalWrite(sensor11, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor11_input
			sensor11_input;
		#else
			pinMode(sensor11, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor11_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor11_output
			sensor11_output;
		#else
			pinMode(sensor11, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor12_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor12)
	// define "sensor12_read" in main program to enable this function, see example
	// define also "sensor12_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_12() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor12_low
			sensor12_low;
		#else
			digitalWrite(sensor12, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor12_input
			sensor12_input;
		#else
			pinMode(sensor12, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor12_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor12_output
			sensor12_output;
		#else
			pinMode(sensor12, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor13_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor13)
	// define "sensor13_read" in main program to enable this function, see example
	// define also "sensor13_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_13() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor13_low
			sensor13_low;
		#else
			digitalWrite(sensor13, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor13_input
			sensor13_input;
		#else
			pinMode(sensor13, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor13_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor13_output
			sensor13_output;
		#else
			pinMode(sensor13, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor14_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor14)
	// define "sensor14_read" in main program to enable this function, see example
	// define also "sensor14_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_14() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor14_low
			sensor14_low;
		#else
			digitalWrite(sensor14, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor14_input
			sensor14_input;
		#else
			pinMode(sensor14, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor14_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor14_output
			sensor14_output;
		#else
			pinMode(sensor14, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor15_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor15)
	// define "sensor15_read" in main program to enable this function, see example
	// define also "sensor15_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_15() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor15_low
			sensor15_low;
		#else
			digitalWrite(sensor15, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor15_input
			sensor15_input;
		#else
			pinMode(sensor15, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor15_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor15_output
			sensor15_output;
		#else
			pinMode(sensor15, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

#ifdef sensor16_read
	// this is the hard-coded (faster) version replacing digitalTouchRead(sensor16)
	// define "sensor16_read" in main program to enable this function, see example
	// define also "sensor16_input/output/low/high" in order to avoid Arduino IO functions
	uint8_t digitalTouchRead_16() // for detailed comments see function digitalTouchRead()
	{
		#ifdef sensor16_low
			sensor16_low;
		#else
			digitalWrite(sensor16, LOW);
		#endif
		uint8_t cycleCounter = 1;
		noInterrupts();
		#ifdef sensor16_input
			sensor16_input;
		#else
			pinMode(sensor16, INPUT);
		#endif
		// here the hard-coded direct port reading appears
		while (!sensor16_read && cycleCounter) cycleCounter++;
		interrupts();
		#ifdef sensor16_output
			sensor16_output;
		#else
			pinMode(sensor16, OUTPUT);
		#endif
		return --cycleCounter;
	}
#endif

// function digitalTouchRead
// takes one sample of measurement of the specified sensor
// works in stabel and well earthed environments, otherwise please use filter methods
// digitalTouchAverage or digitalTouchMedian for a more reliable result
uint8_t digitalTouchRead(uint8_t pin)
{
	// if hard-coded funtions exist, use them!
	#ifdef sensor1_read
		if (pin == sensor1) return digitalTouchRead_1();
	#endif
	#ifdef sensor2_read
		if (pin == sensor2) return digitalTouchRead_2();
	#endif
	#ifdef sensor3_read
		if (pin == sensor3) return digitalTouchRead_3();
	#endif
	#ifdef sensor4_read
		if (pin == sensor4) return digitalTouchRead_4();
	#endif
	#ifdef sensor5_read
		if (pin == sensor5) return digitalTouchRead_5();
	#endif
	#ifdef sensor6_read
		if (pin == sensor6) return digitalTouchRead_6();
	#endif
	#ifdef sensor7_read
		if (pin == sensor7) return digitalTouchRead_7();
	#endif
	#ifdef sensor8_read
		if (pin == sensor8) return digitalTouchRead_8();
	#endif
	#ifdef sensor9_read
		if (pin == sensor9) return digitalTouchRead_9();
	#endif
	#ifdef sensor10_read
		if (pin == sensor10) return digitalTouchRead_10();
	#endif
	#ifdef sensor11_read
		if (pin == sensor11) return digitalTouchRead_11();
	#endif
	#ifdef sensor12_read
		if (pin == sensor12) return digitalTouchRead_12();
	#endif
	#ifdef sensor13_read
		if (pin == sensor13) return digitalTouchRead_13();
	#endif
	#ifdef sensor14_read
		if (pin == sensor14) return digitalTouchRead_14();
	#endif
	#ifdef sensor15_read
		if (pin == sensor15) return digitalTouchRead_15();
	#endif
	#ifdef sensor16_read
		if (pin == sensor16) return digitalTouchRead_16();
	#endif
	
	// the rest of the function is only used if there is a sensor left that is used but has no
	// definition for sensorx_read
	#if \
	 (defined sensor1 & !defined sensor1_read) || \
	 (defined sensor2 & !defined sensor2_read) || \
	 (defined sensor3 & !defined sensor3_read) || \
	 (defined sensor4 & !defined sensor4_read) || \
	 (defined sensor5 & !defined sensor5_read) || \
	 (defined sensor6 & !defined sensor6_read) || \
	 (defined sensor7 & !defined sensor7_read) || \
	 (defined sensor8 & !defined sensor8_read) || \
	 (defined sensor9 & !defined sensor9_read) || \
	 (defined sensor10 & !defined sensor10_read) || \
	 (defined sensor11 & !defined sensor11_read) || \
	 (defined sensor12 & !defined sensor12_read) || \
	 (defined sensor13 & !defined sensor13_read) || \
	 (defined sensor14 & !defined sensor14_read) || \
	 (defined sensor15 & !defined sensor15_read) || \
	 (defined sensor16 & !defined sensor16_read)
		// discharge sensor cap by driving a LOW signal
		digitalWrite(pin, LOW);

		// loop counter to measure the charging time, start at 1, 0 is overflow
		uint8_t cycleCounter = 1;
		
		// Loops are faster (higher resolution) if we use direct read and get following values outside the loop.
		// However, still variables are used and it is not as fast as the hard-coded version using sensorx_read.
		volatile uint8_t *inputRegister = portInputRegister(digitalPinToPort(pin));
		uint8_t inputMask = digitalPinToBitMask(pin);

		// with interrupts during the measurement we would miss a lot of counts
		noInterrupts();
		
		// switch off driver, sensor cap starts to charge through external resistor
		pinMode(pin, INPUT);
		
		// charge the sensor until signal is HIGH or counter is 0 (= overflow)
		// this loop must be fast in order to get a good resolution -> direct port reading is used
		// the term "(*inputRegister & inputMask)" is a non-zero byte if the input is HIGH
		// (the hard-coded functions digitalTouchRead_x() replace the variables by constants and are even faster)
		while (!(*inputRegister & inputMask) && cycleCounter) cycleCounter++;

		// measuring loop is done, interrupts are allowed again
		interrupts();

		// discharge sensor cap (or use pin for LED, see example sketch)
		pinMode(pin, OUTPUT);

		// reduce cycleCounter by 1 since it started at 1, on overflow it is zero and will become 255 then
		return --cycleCounter;
	#endif
}


// function digitalTouchAverage
// take the average of a number of samples
// This method is useful, if you want to have a high number of samples for best results.
// It also can be used with samples = 1 (default), it will add one prior sample, which is
// ignored for better stability after the pin has been used for a LED.
uint8_t digitalTouchAverage(uint8_t pin, uint8_t samples = 1)
{
	uint16_t value = 0;
	
	// ignore first sample
	digitalTouchRead(pin);

	// read specified number of samples and add result
	for (uint8_t i = 0; i < samples; i++)
	{
		value += (uint16_t)digitalTouchRead(pin);
	}

	// return average
	return (uint8_t)(value / (uint16_t)samples);
}


// function digitalTouchMedian
// take the median of three samples
// The median is more useful for a small number of samples (3 in this case), because it ignores
// single outliers completely while the average would be strongly affected.
// It also adds one prior sample, which is ignored for better stability after the pin has been used
// for a LED. 
uint8_t digitalTouchMedian(uint8_t pin)
{
	// first sample is ignored
	digitalTouchRead(pin);

	// take three samples
	uint8_t value0 = digitalTouchRead(pin);
    uint8_t value1 = digitalTouchRead(pin);
    uint8_t value2 = digitalTouchRead(pin);

	// find the median, which is the middle value in a sorted list, no value is modified
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

// function sensorLEDsOff
// switches all sensor output ports to low
// this must be done for all pins before the first sensor is sampled, so we need this extra function
// if no LEDs are used, this may not be called ever
void sensorLEDsOff()
{
  #ifdef sensor1_low
    sensor1_low;
  #elif defined sensor1
    digitalWrite(sensor1, LOW);
  #endif
  #ifdef sensor2_low
    sensor2_low;
  #elif defined sensor2
    digitalWrite(sensor2, LOW);
  #endif
  #ifdef sensor3_low
    sensor3_low;
  #elif defined sensor3
    digitalWrite(sensor3, LOW);
  #endif
  #ifdef sensor4_low
    sensor4_low;
  #elif defined sensor4
    digitalWrite(sensor4, LOW);
  #endif
  #ifdef sensor5_low
    sensor5_low;
  #elif defined sensor5
    digitalWrite(sensor5, LOW);
  #endif
  #ifdef sensor6_low
    sensor6_low;
  #elif defined sensor6
    digitalWrite(sensor6, LOW);
  #endif
  #ifdef sensor7_low
    sensor7_low;
  #elif defined sensor7
    digitalWrite(sensor7, LOW);
  #endif
  #ifdef sensor8_low
    sensor8_low;
  #elif defined sensor8
    digitalWrite(sensor8, LOW);
  #endif
  #ifdef sensor9_low
    sensor9_low;
  #elif defined sensor9
    digitalWrite(sensor9, LOW);
  #endif
  #ifdef sensor10_low
    sensor10_low;
  #elif defined sensor10
    digitalWrite(sensor10, LOW);
  #endif
  #ifdef sensor11_low
    sensor11_low;
  #elif defined sensor11
    digitalWrite(sensor11, LOW);
  #endif
  #ifdef sensor12_low
    sensor12_low;
  #elif defined sensor12
    digitalWrite(sensor12, LOW);
  #endif
  #ifdef sensor13_low
    sensor13_low;
  #elif defined sensor13
    digitalWrite(sensor13, LOW);
  #endif
  #ifdef sensor14_low
    sensor14_low;
  #elif defined sensor14
    digitalWrite(sensor14, LOW);
  #endif
  #ifdef sensor15_low
    sensor15_low;
  #elif defined sensor15
    digitalWrite(sensor15, LOW);
  #endif
  #ifdef sensor16_low
    sensor16_low;
  #elif defined sensor16
    digitalWrite(sensor16, LOW);
  #endif
}
