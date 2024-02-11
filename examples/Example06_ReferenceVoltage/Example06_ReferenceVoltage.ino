/*
  Configuring the ADS1219 ADC voltage reference.

  By default, the ADS1219 uses an internal 2.048V voltage reference for each conversion.
  This means, by default, the voltage measurements are limited to +/-2.048V.
  But it can also be configured to use the REFP and REFN pins, allowing the reference
  volatge to be defined by the user.
  In this example, use jumper wires to set REFP to VDDA (3.3V) and REFN to 0V (GND).
  This allows voltage measurements in the range +/-3.3V.

  By: Paul Clark
  SparkFun Electronics
  Date: 2024/02/11
  SparkFun code, firmware, and software is released under the MIT License.
  Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADS1219
  QWIIC        --> QWIIC

  REFP and REFN:
  Use jumper wires or jumper links (https://www.sparkfun.com/products/9044) to connect:
  REFP --> VDDA (3.3V)
  REFN --> GND

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
  }
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
  myADC.setInputMultiplexer(ADS1219_CONFIG_MUX_DIFF_P0_N1); // Read the differential voltage between AIN0 (+) and AIN1 (-)

  // Configure the voltage reference. Options are:
  // ADS1219_VREF_INTERNAL (The internal 2.048V reference. Default)
  // ADS1219_VREF_EXTERNAL (The REFP and REFN pins)
  // Note: for the external reference, we need to tell getConversionMillivolts what the reference voltage is. See below.
  myADC.setVoltageReference(ADS1219_VREF_EXTERNAL); // Use the REFP and REFN pins

  Serial.println("ADC initialized");
}

void loop()
{
  myADC.startSync(); // Start a single-shot conversion.

  while (myADC.dataReady() == false) // Check if the conversion is complete. This will return true if data is ready.
  {
    delay(10); // The conversion is not complete. Wait a little to avoid pounding the I2C bus.
  }

  myADC.readConversion(); // Read the conversion result from the ADC. Store it internally.
  float milliVolts = myADC.getConversionMillivolts(3300.0); // Convert to millivolts - using a reference of 3300mV (3.3V)
  Serial.print("ADC voltage (mV): ");
  Serial.println(milliVolts, 3); // Print milliVolts with 3 decimal places
}
