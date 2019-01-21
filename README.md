# DigitalTouch 1.1.0

This library lets you measure the capacitive touch of an Arduino pin by measuring the charging-time
of the sensor cap through an external high-ohmic resistor. An LED can be connected to the same pin.
 - no ADC, timer, comparator or IRQ is required
 - one pin per channel, no extra send-pin
 - one LED can be added to each channel without extra pins and can be controlled independently
 - one external resistor is required, a second resistor is optional for ESD robustness and if LEDs are used
 - easy to adapt to every controller, only general I/Os are used (Smitt-Trigger input)

The intention of the code is to run on very small controllers.

The sensor should be isolated. The circuit should be grounded.
 
For wiring and other information please read the extensive documentation in the library file.

Installation
============
Download the zip, extract and remove the "-master" of the folder.
Install the library as described here: http://arduino.cc/en/pmwiki.php?n=Guide/Libraries

Credits/Links
=============
The example sketch and the library structure and filtering is based on the AnalogTouch library
by Nico Hood:
https://github.com/NicoHood/AnalogTouch

The measuring method was changed from ADC to digital-based. A median filter was added as a second
filter option.

Version History
===============
1.1.0 Release (21.01.2019)
* adding hard-coded IO function, needs new #define statements but is fully optional
* adding function sensorLEDsOff()

1.0.0 Release (17.01.2019)
* Initial Release

License and Copyright
=====================
Copyright (c) 2019 Rainer Urlacher
under the MIT License
