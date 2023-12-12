/*
  Configuring the ADS1219 ADC input multiplexer.

  The ADS1219 has an input multiplexer which allows it to read:
  single-ended voltages from AIN0/1/2/3;
  differential voltages from AIN0-AIN1, AIN2-AIN3, AIN1-AIN2;
  or the input can be shorted (to AVDD divided by two) for offset measurement.
  This example shows how to configure the multiplexer and then read the voltage.

  By: Paul Clark
  SparkFun Electronics
  Date: 2023/12/11
  SparkFun code, firmware, and software is released under the MIT License.
  Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADS1219
  QWIIC        --> QWIIC

  Open the serial monitor at 115200 baud to see the voltage.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/23455 - Qwiic ADS1219 1x1
*/

// You will need the SparkFun Toolkit. Click here to get it: http://librarymanager/All#SparkFun_Toolkit

#include <SparkFun_ADS1219.h> // Click here to get the library: http://librarymanager/All#SparkFun_ADS1219

SfeADS1219ArdI2C myADC;

void setup()
{

  delay(1000); // Allow time for the microcontroller to start up

  Serial.begin(115200); // Begin the Serial console
  while (!Serial)
  {
    delay(100); // Wait for the user to open the Serial Monitor
  };
  Serial.println("SparkFun ADS1219 Example");

  Wire.begin(); // Begin the I2C bus

  // Initialize ADC - this also performs a soft reset
  while (myADC.begin() == false)
  {
    Serial.println("ADC failed to begin. Please check your wiring! Retrying...");
    delay(1000);
  }

  // Configure the input multiplexer. Options are:
  // ADS1219_CONFIG_MUX_DIFF_P0_N1 (Default)
  // ADS1219_CONFIG_MUX_DIFF_P2_N3
  // ADS1219_CONFIG_MUX_DIFF_P1_N2
  // ADS1219_CONFIG_MUX_SINGLE_0
  // ADS1219_CONFIG_MUX_SINGLE_1
  // ADS1219_CONFIG_MUX_SINGLE_2
  // ADS1219_CONFIG_MUX_SINGLE_3
  // ADS1219_CONFIG_MUX_SHORTED
  myADC.setInputMultiplexer(ADS1219_CONFIG_MUX_SINGLE_0); // Read the voltage between AIN0 and GND

  Serial.println("ADC initialized");

  Serial.println("Reading the voltage through the input multiplexer");
}

void loop()
{
  myADC.startSync(); // Start a single-shot conversion.

  while (myADC.dataReady() == false) // Check if the conversion is complete. This will return true if data is ready.
  {
    delay(10); // The conversion is not complete. Wait a little to avoid pounding the I2C bus.
  }

  myADC.readConversion(); // Read the conversion result from the ADC. Store it internally.
  float milliVolts = myADC.getConversionMillivolts(); // Convert to millivolts.
  Serial.print("ADC voltage (mV): ");
  Serial.println(milliVolts, 3); // Print milliVolts with 3 decimal places
}
