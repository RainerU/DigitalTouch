/*
Copyright (c) 2019 Rainer Urlacher

DigitalTouch example

Two capacitive sensors using digitial IO pins.
Control LEDs connected to the same pins.

Code is adapted from AnalogTouch example sketch by NicoHood.
https://github.com/NicoHood/AnalogTouch

for more comments see the documentation in the library file DigitalTouch.h


How to define your sensor/LED pins:
-----------------------------------

This library is intended for very small controllers which may be slow and have only a few
10 bytes of RAM. So a lot of focus was put on the optimization of speed and code size.

You can define sensor1 .. sensor16, other names cannot be handled by the library (would be
easy to extend).

You can (but don't need to) define sensor1_read .. sensor16_read. This creates separate code
for the corresponding sensor that makes the execution faster. The resolution of the time-measuring
loop becomes a bit better this way. On the other hand, if you have multiple sensors, a bit more code
will be generated.

You can (but don't need to) define sensorx_read, sensorx_input, sensorx_output, sensorx_high, sensorx_low
for EACH sensor. Doing this allows you to avoid digitalWrite/Read and pinMode in the entire program.
Removing these functions reduces the size of the Arduino core code significantly.

How to find the right values for sensorx_read (x is the number 1..16 of the sensor):
------------------------------------------------------------------------------------
- search for "pin mapping" or look into the schematic to find port names according to pin numbers
- if e.g. the port is PC3, you must use the predefined name PINC to get the right port register
- then you must "AND" this register with a binary value that selects the bit of interest
- for PC3 you select Bit 3, the most right one is Bit 0, so you use the binary value B00001000
result is:
#define sensorx_read (PINC & B00001000)
This definition hard-codes the port reading. The compiler can put constants into the program
instead of reading from variables.

If you do not want this, just do not define "sensorx_read", and the library will automagically
calculate everything from the pin number "sensorx". Even in this case the Arduino function
digitalRead() is not used, but the register and bitmask must be stored in variables.

Make sure, that you do not define a sensor that is not used.

Defining sensorx_input, sensorx_output, sensorx_high, sensorx_low is very similar. Sometimes the binary
value has to be inverted. Just refer to the following examples:

*/

// remove comment signs from the following #define statements to get the most optimized code:

// Arduino Mega: Pin 53 = Port PB0
#define sensor1   53                                 // Arduino pin number
//#define sensor1_read   (PINB & B00000001)            // replacing digitalRead(sensor1)
//#define sensor1_input  DDRB = DDRB & B11111110       // replacing pinMode(sensor1, INPUT)
//#define sensor1_output DDRB = DDRB | B00000001       // replacing pinMode(sensor1, OUTPUT)
//#define sensor1_low    PORTB = PORTB & B11111110     // replacing digitalWrite(sensor1, LOW)
//#define sensor1_high   PORTB = PORTB | B00000001     // replacing digitalWrite(sensor1, HIGH)

//
// Arduino Mega: Pin 51 = Port PB2
#define sensor2   51
//#define sensor2_read   (PINB & B00000100)
//#define sensor2_input  DDRB = DDRB & B11111011
//#define sensor2_output DDRB = DDRB | B00000100
//#define sensor2_low    PORTB = PORTB & B11111011
//#define sensor2_high   PORTB = PORTB | B00000100

// DigitalTouch library
#include <DigitalTouch.h>

// level of filtering for self calibration
#define offset 4
#if offset > 8
#error "Too big offset value"
#endif

// number of samples averaged for each measurement
#define samples 5
#if samples > 255
#error "Too many samples"
#endif

// number of additional timing loops over baseline that indicate a touched sensor
#define sensorThreshold 4

void setup()
{
  // Start Serial for debugging
  Serial.begin(115200);
}

void loop()
{
  static uint16_t ref1 = 0xFFFF;
  static uint16_t ref2 = 0xFFFF;

  // all LEDs connected to sensors must be off before measuring the first sensor, not visible
  // not needed if you don't use LEDs at sensor pins
  sensorLEDsOff();

  // read sensor 1 and filter samples with the average method
  uint8_t value1 = digitalTouchAverage(sensor1, samples);
  // read sensor 2 and filter three samples with the median method (just for a different example)
  uint8_t value2 = digitalTouchMedian(sensor2);

  // capture all sensors
  bool touched1 = (value1 - (uint8_t)(ref1 >> offset)) > sensorThreshold;
  bool touched2 = (value2 - (uint8_t)(ref2 >> offset)) > sensorThreshold;

  // LEDs on when touched (LEDs can be used for everything, not limited to sensor results)
  // you can remove the ifdef/else and just write the version that you want to use
  #ifdef sensor1_high
    if (touched1) sensor1_high;
  #else
    digitalWrite(sensor1, touched1);
  #endif
  #ifdef sensor2_high
    if (touched2) sensor2_high;
  #else
    digitalWrite(sensor2, touched2);
  #endif

  // Print touched?
  Serial.print(touched1);
  Serial.print("\t");

  // Print calibrated value
  Serial.print(value1 - (uint8_t)(ref1 >> offset));
  Serial.print("\t");

  // Print raw value
  Serial.print(value1);
  Serial.print("\t");

  // Print raw ref
  Serial.print(ref1 >> offset);
  Serial.print("\t");
  Serial.print(ref1);

  Serial.print("\t\t");
  
  // Print touched?
  Serial.print(touched2);
  Serial.print("\t");

  // Print calibrated value
  Serial.print(value2 - (uint8_t)(ref2 >> offset));
  Serial.print("\t");

  // Print raw value
  Serial.print(value2);
  Serial.print("\t");

  // Print raw ref
  Serial.print(ref2 >> offset);
  Serial.print("\t");
  Serial.println(ref2);

  // Self calibrate
  if (value1 <= (uint8_t)(ref1 >> offset))
    ref1 = ((uint16_t)value1 << offset);
  // Cool down
  else ref1++;

  if (value2 <= (uint8_t)(ref2 >> offset))
    ref2 = ((uint16_t)value2 << offset);
  // Cool down
  else ref2++;

  // Wait some time
  delay(100);
}
