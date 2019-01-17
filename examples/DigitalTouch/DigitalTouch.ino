/*
 Copyright (c) 2019 Rainer Urlacher

 DigitalTouch example
 
 Two capacitive sensors using digitial IO pins.
 Control LEDs connected to the same pins.

 Code is adapted from AnalogTouch example sketch by NicoHood.
 https://github.com/NicoHood/AnalogTouch

 for more comments see the documentation in the library file DigitalTouch.h
 
*/

// DigitalTouch
#include <DigitalTouch.h>

// Choose your sensor/led pins
#define sensor1   53
#define sensor2   51

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

  // all LEDs connected to sensors must be off before measuring sensors, not visible
  digitalWrite(sensor1, LOW);
  digitalWrite(sensor2, LOW);

  // read sensor 1 and filter samples with the average method
  uint8_t value1 = digitalTouchAverage(sensor1, samples);
  // read sensor 2 and filter three samples with the median method (just a different example)
  uint8_t value2 = digitalTouchMedian(sensor2);

  // capture all sensors
  bool touched1 = (value1 - (uint8_t)(ref1 >> offset)) > sensorThreshold;
  bool touched2 = (value2 - (uint8_t)(ref2 >> offset)) > sensorThreshold;

  // LEDs on when touched (LEDs can be used for everything, not limited to sensor results)
  digitalWrite(sensor1, touched1);
  digitalWrite(sensor2, touched2);

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
